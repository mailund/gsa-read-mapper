

/*
 Readmapper based on Burrows-Wheeler transform search.
 */

#include "suffix_array.h"
#include "fasta.h"
#include "fastq.h"
#include "sam.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>

struct search_info {
    int edit_dist;
    struct fasta_records *records;
    FILE *sam_file;
};

static struct search_info *empty_search_info()
{
    struct search_info *info =
        (struct search_info*)malloc(sizeof(struct search_info));
    info->edit_dist = 0;
    info->records = empty_fasta_records();
    return info;
}

static void delete_search_info(struct search_info *info)
{
    delete_fasta_records(info->records);
    free(info);
}

struct read_search_info {
    const char *ref_name;
    const char *read_name;
    const char *read;
    const char *quality;
    struct search_info *search_info;
};

static struct read_search_info *empty_read_search_info()
{
    struct read_search_info *info =
        (struct read_search_info*)malloc(sizeof(struct read_search_info));
    
    info->ref_name = 0;
    info->read_name = 0;
    info->read = 0;
    info->quality = 0;
    info->search_info = 0;

    return info;
}

static void delete_read_search_info(struct read_search_info *info)
{
    free(info);
}

static void read_callback(const char *read_name,
                          const char *read,
                          const char *quality,
                          void * callback_data) {
    struct search_info *search_info = (struct search_info*)callback_data;
    
    // I allocate and deallocate the info all the time... I might
    // be able to save some time by not doing this, but compared to
    // building and removeing the trie, I don't think it will be much.
    struct read_search_info *info = empty_read_search_info();
    info->search_info = search_info;
    info->read = read;
    info->quality = quality;
    info->read_name = read_name;
    
    // FIXME: SEARCH HERE
    
    delete_read_search_info(info);
}

int main(int argc, char * argv[])
{
    const char *prog_name = argv[0];

    int opt; bool preprocess = false; int edit_dist = 0;
    static struct option longopts[] = {
        { "help",        no_argument,            NULL,           'h' },
        { "preprocess",  required_argument,      NULL,           'p' },
        { "distance",    required_argument,      NULL,           'd' },
        { NULL,         0,                      NULL,            0  }
    };
    while ((opt = getopt_long(argc, argv, "hpd:", longopts, NULL)) != -1) {
        switch (opt) {
            case 'h':
                printf("Usage: %s -p | --preprocess ref.fa\n"
                       "       %s [-d distance] ref.fa reads.fq\n",
                       prog_name, prog_name);
                printf("Options:\n");
                printf("\t-h | --help:\t\t Show this message.\n");
                printf("\t-d | --distance:\t Maximum edit distance for the search.\n");
                printf("\t-p | --preprocess:\t Preprocess a reference genome.\n");
                printf("\n\n");
                return EXIT_SUCCESS;
                
            case 'p':
                preprocess = true;
                break;

            case 'd':
                edit_dist = atoi(optarg);
                break;
                
            default:
                fprintf(stderr,
                        "Usage: %s -p|--preprocess ref.fa\n"
                        "       %s [-d distance] ref.fa reads.fq\n",
                        prog_name, prog_name);
                return EXIT_FAILURE;
        }
    }
    argc -= optind;
    argv += optind;

    if (preprocess) {
        if (argc != 1) {
            fprintf(stderr,
                    "Usage: %s -p|--preprocess ref.fa\n"
                    "       %s [-d distance] ref.fa reads.fq\n",
                    prog_name, prog_name);
            return EXIT_FAILURE;
        }
        
        FILE *fasta_file = fopen(argv[0], "r");
        if (!fasta_file) {
            fprintf(stderr, "Could not open %s.\n", argv[0]);
            return EXIT_FAILURE;
        }
        
        struct fasta_records *records = empty_fasta_records();
        read_fasta_records(records, fasta_file);
        fclose(fasta_file);
        
        // FIXME: preprocess
        for (int i = 0; i < records->names->used; i++) {
            struct suffix_array *sa = qsort_sa_construction(records->sequences->strings[i]);
            printf("%s", records->names->strings[i]);
            for (int j = 0; j < sa->length; j ++) {
                printf(" %lu", sa->array[i]);
            }
            printf("\n");
            delete_suffix_array(sa);
        }
        
        delete_fasta_records(records);
        
    } else {
        if (argc != 2) {
            fprintf(stderr,
                    "Usage: %s -p|--preprocess ref.fa\n"
                    "       %s [-d distance] ref.fa reads.fq\n",
                    prog_name, prog_name);
            return EXIT_FAILURE;
        }
    
        FILE *fasta_file = fopen(argv[0], "r");
        if (!fasta_file) {
            fprintf(stderr, "Could not open %s.\n", argv[0]);
            return EXIT_FAILURE;
        }
        
        FILE *fastq_file = fopen(argv[1], "r");
        if (!fastq_file) {
            fprintf(stderr, "Could not open %s.\n", argv[1]);
            return EXIT_FAILURE;
        }
    
        struct search_info *search_info = empty_search_info();
        search_info->edit_dist = edit_dist;
        read_fasta_records(search_info->records, fasta_file);
        fclose(fasta_file);
        
        search_info->sam_file = stdout;
        
        scan_fastq(fastq_file, read_callback, search_info);

        delete_search_info(search_info);
        fclose(fastq_file);

    }
        
    return EXIT_SUCCESS;
}