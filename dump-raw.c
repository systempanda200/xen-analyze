#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <assert.h>
#include "trace.h"

typedef unsigned long long tsc_t;

struct {
    int fd;
    int cur_cpu;
    char * trace_file;
} G = {
    .fd=-1,
    .cur_cpu = -1,
    .trace_file = NULL,
};

/* -- on-disk trace buffer definitions -- */
struct trace_record {
    unsigned event:28,
        extra_words:3,
        cycle_flag:1;
    union {
        struct {
            uint32_t tsc_lo, tsc_hi;
            uint32_t data[7];
        } tsc;
        struct {
            uint32_t data[7];
        } notsc;
    } u;
};

struct record_info {
    int cpu;
    tsc_t tsc;
    union {
        unsigned event;
        struct {
            unsigned minor:12,
                sub:4,
                main:12,
                unused:4;
        } evt;
    };
    int extra_words;
    int size;
    uint32_t *d;

    struct trace_record rec;
};

struct data {
    loff_t file_offset, next_cpu_change_offset;
    int pid;
    struct record_info ri;
};

static inline ssize_t get_rec_size(struct trace_record *rec) {
    ssize_t s;
    
    s = sizeof(uint32_t);
        
    if(rec->cycle_flag)
        s += sizeof(tsc_t);

    s += rec->extra_words * sizeof(uint32_t);

    return s;
}

ssize_t __read_record(int fd, struct trace_record *rec, loff_t offset)
{
    ssize_t r, rsize;

    r=pread64(G.fd, rec, sizeof(*rec), offset);

    if(r < 0) {
        /* Read error */
        perror("read");
        fprintf(stderr, "offset %llx\n", (unsigned long long)offset);
        return 0;
    } else if(r==0) {
        /* End-of-file */
        return 0;
    } else if(r < sizeof(uint32_t)) {
        /* Full header not read */
        fprintf(stderr, "%s: short read (%zd bytes)\n",
                __func__, r);
        exit(1);
    }

    rsize=get_rec_size(rec);

    if(r < rsize) {
        /* Full record not read */
        fprintf(stderr, "%s: short read (%zd, expected %zd)\n",
                __func__, r, rsize);
        return 0;
    }

    return rsize;
}

void __fill_in_record_info(struct record_info *ri)
{
    tsc_t tsc=0;

    ri->event = ri->rec.event;
    ri->extra_words = ri->rec.extra_words;

    if(ri->rec.cycle_flag) {
        tsc = (((tsc_t)ri->rec.u.tsc.tsc_hi) << 32)
                | ri->rec.u.tsc.tsc_lo;

        ri->tsc = tsc;
        ri->d = ri->rec.u.tsc.data;
    } else {
        ri->tsc = 0;
        ri->d = ri->rec.u.notsc.data;
    }
}

struct cpu_change_data {
    int cpu;
    unsigned window_size;
};

void process_cpu_change(struct data *p) {
    struct record_info *ri = &p->ri;
    struct cpu_change_data *r = (typeof(r))ri->d;

    printf(" cpu_change this-cpu %u record-cpu %u window_size %u(0x%08x)\n",
           p->pid, r->cpu, r->window_size,
           r->window_size);

    /* File sanity check */
    if(p->file_offset != p->next_cpu_change_offset) {
        printf("Strange, pcpu %d expected offet %llx, actual %llx!\n",
	       p->pid, (unsigned long long)p->next_cpu_change_offset,
	       (unsigned long long)p->file_offset);
    }

    p->next_cpu_change_offset = p->file_offset + ri->size + r->window_size;

    p->pid = r->cpu;
}

void print_raw_record(struct data *p) {
    struct record_info *ri = &p->ri;
    int i;
    
    printf("R p%2d o%016llx %8lx %d ",
           p->pid, (unsigned long long)p->file_offset,
           (unsigned long)ri->rec.event, ri->rec.extra_words);

    if(ri->rec.cycle_flag)
        printf("t%016llx [ ", ri->tsc);
    else
        printf("                  [ ");

    for(i=0; i<ri->rec.extra_words; i++)
        printf("%x ",
               ri->d[i]);
    printf("]\n");
}

void read_records(int fd) {
    struct data p = {
        .file_offset = 0,
        .next_cpu_change_offset = 0,
        .pid = -1,
        .ri = { 0 }
    };
    struct record_info *ri=&p.ri;

    do {
        ri->size = __read_record(fd, &ri->rec, p.file_offset);

        if(ri->size)
        {
            __fill_in_record_info(ri);

            print_raw_record(&p);

            if(ri->event == TRC_TRACE_CPU_CHANGE)
                process_cpu_change(&p);

            p.file_offset += ri->size;
        }
    } while ( ri->size );
}

int main(int argc, char * argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s tracefile\n", argv[0]);
        exit(1);
    }

    G.fd = open(argv[1], O_RDONLY|O_LARGEFILE);

    read_records(G.fd);
}
