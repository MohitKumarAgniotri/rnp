/*
 * Copyright (c) 2017, [Ribose Inc](https://www.ribose.com).
 * Copyright (c) 2009 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is originally derived from software contributed to
 * The NetBSD Foundation by Alistair Crooks (agc@netbsd.org), and
 * carried further by Ribose Inc (https://www.ribose.com).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/* Command line program to perform rnp operations */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <errno.h>
#include <getopt.h>
#include <regex.h>
#include <rnp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* SHA1 is now looking as though it should not be used.  Let's
 * pre-empt this by specifying SHA256 - gpg interoperates just fine
 * with SHA256 - agc, 20090522
 */
#define DEFAULT_HASH_ALG "SHA256"

extern char *__progname;

static const char *usage = "--help OR\n"
                           "\t--encrypt [--output=file] [options] files... OR\n"
                           "\t--decrypt [--output=file] [options] files... OR\n\n"
                           "\t--sign [--detach] [--hash=alg] [--output=file]\n"
                           "\t\t[options] files... OR\n"
                           "\t--verify [options] files... OR\n"
                           "\t--cat [--output=file] [options] files... OR\n"
                           "\t--clearsign [--output=file] [options] files... OR\n"
                           "\t--list-packets [options] OR\n"
                           "\t--version\n"
                           "where options are:\n"
                           "\t[--armor] AND/OR\n"
                           "\t[--cipher=<ciphername>] AND/OR\n"
                           "\t[--coredumps] AND/OR\n"
                           "\t[--homedir=<homedir>] AND/OR\n"
                           "\t[--keyring=<keyring>] AND/OR\n"
                           "\t[--keyring-format=<format>] AND/OR\n"
                           "\t[--numtries=<attempts>] AND/OR\n"
                           "\t[--userid=<userid>] AND/OR\n"
                           "\t[--maxmemalloc=<number of bytes>] AND/OR\n"
                           "\t[--verbose]\n";

enum optdefs {
    /* commands */
    ENCRYPT = 260,
    DECRYPT,
    SIGN,
    CLEARSIGN,
    VERIFY,
    VERIFY_CAT,
    LIST_PACKETS,
    SHOW_KEYS,
    VERSION_CMD,
    HELP_CMD,

    /* options */
    SSHKEYS,
    KEYRING,
    KEYRING_FORMAT,
    USERID,
    ARMOUR,
    HOMEDIR,
    DETACHED,
    HASH_ALG,
    OUTPUT,
    RESULTS,
    VERBOSE,
    COREDUMPS,
    PASSWDFD,
    SSHKEYFILE,
    MAX_MEM_ALLOC,
    DURATION,
    BIRTHTIME,
    CIPHER,
    NUMTRIES,

    /* debug */
    OPS_DEBUG
};

#define EXIT_ERROR 2

static struct option options[] = {
  /* file manipulation commands */
  {"encrypt", no_argument, NULL, ENCRYPT},
  {"decrypt", no_argument, NULL, DECRYPT},
  {"sign", no_argument, NULL, SIGN},
  {"clearsign", no_argument, NULL, CLEARSIGN},
  {"verify", no_argument, NULL, VERIFY},
  {"cat", no_argument, NULL, VERIFY_CAT},
  {"vericat", no_argument, NULL, VERIFY_CAT},
  {"verify-cat", no_argument, NULL, VERIFY_CAT},
  {"verify-show", no_argument, NULL, VERIFY_CAT},
  {"verifyshow", no_argument, NULL, VERIFY_CAT},
  /* file listing commands */
  {"list-packets", no_argument, NULL, LIST_PACKETS},
  /* debugging commands */
  {"help", no_argument, NULL, HELP_CMD},
  {"version", no_argument, NULL, VERSION_CMD},
  {"debug", required_argument, NULL, OPS_DEBUG},
  {"show-keys", no_argument, NULL, SHOW_KEYS},
  {"showkeys", no_argument, NULL, SHOW_KEYS},
  /* options */
  {"ssh", no_argument, NULL, SSHKEYS},
  {"ssh-keys", no_argument, NULL, SSHKEYS},
  {"sshkeyfile", required_argument, NULL, SSHKEYFILE},
  {"coredumps", no_argument, NULL, COREDUMPS},
  {"keyring", required_argument, NULL, KEYRING},
  {"keyring-format", required_argument, NULL, KEYRING_FORMAT},
  {"userid", required_argument, NULL, USERID},
  {"home", required_argument, NULL, HOMEDIR},
  {"homedir", required_argument, NULL, HOMEDIR},
  {"ascii", no_argument, NULL, ARMOUR},
  {"armor", no_argument, NULL, ARMOUR},
  {"armour", no_argument, NULL, ARMOUR},
  {"detach", no_argument, NULL, DETACHED},
  {"detached", no_argument, NULL, DETACHED},
  {"hash-alg", required_argument, NULL, HASH_ALG},
  {"hash", required_argument, NULL, HASH_ALG},
  {"algorithm", required_argument, NULL, HASH_ALG},
  {"verbose", no_argument, NULL, VERBOSE},
  {"pass-fd", required_argument, NULL, PASSWDFD},
  {"output", required_argument, NULL, OUTPUT},
  {"results", required_argument, NULL, RESULTS},
  {"maxmemalloc", required_argument, NULL, MAX_MEM_ALLOC},
  {"max-mem", required_argument, NULL, MAX_MEM_ALLOC},
  {"max-alloc", required_argument, NULL, MAX_MEM_ALLOC},
  {"from", required_argument, NULL, BIRTHTIME},
  {"birth", required_argument, NULL, BIRTHTIME},
  {"birthtime", required_argument, NULL, BIRTHTIME},
  {"creation", required_argument, NULL, BIRTHTIME},
  {"duration", required_argument, NULL, DURATION},
  {"expiry", required_argument, NULL, DURATION},
  {"cipher", required_argument, NULL, CIPHER},
  {"num-tries", required_argument, NULL, NUMTRIES},
  {"numtries", required_argument, NULL, NUMTRIES},
  {"attempts", required_argument, NULL, NUMTRIES},
  {NULL, 0, NULL, 0},
};

/* gather up program variables into one struct */
typedef struct prog_t {
    char  keyring[MAXPATHLEN + 1]; /* name of keyring */
    char *output;                  /* output file name */
    int   overwrite;               /* overwrite files? */
    int   armour;                  /* ASCII armor */
    int   detached;                /* use separate file */
    int   cmd;                     /* rnp command */
} prog_t;

static void
print_praise(void)
{
    fprintf(stderr,
            "%s\nAll bug reports, praise and chocolate, please, to:\n%s\n",
            rnp_get_info("version"),
            rnp_get_info("maintainer"));
}

/* print a usage message */
static void
print_usage(const char *usagemsg)
{
    print_praise();
    fprintf(stderr, "Usage: %s %s", __progname, usagemsg);
}

/* read all of stdin into memory */
static int
stdin_to_mem(rnp_t *rnp, char **temp, char **out, unsigned *maxsize)
{
    unsigned newsize;
    unsigned size;
    char     buf[BUFSIZ * 8];
    char *   loc;
    int      n;

    *maxsize = (unsigned) atoi(rnp_getvar(rnp, "max mem alloc"));
    size = 0;
    *temp = NULL;
    while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        /* round up the allocation */
        newsize = size + ((n / BUFSIZ) + 1) * BUFSIZ;
        if (newsize > *maxsize) {
            fputs("bounds check\n", stderr);
            return size;
        }
        loc = realloc(*temp, newsize);
        if (loc == NULL) {
            fputs("short read\n", stderr);
            return size;
        }
        *temp = loc;
        memcpy(&(*temp)[size], buf, n);
        size += n;
    }
    if ((*out = calloc(1, *maxsize)) == NULL) {
        fputs("Bad alloc\n", stderr);
        return 0;
    }
    return (int) size;
}

/* output the text to stdout */
static int
show_output(char *out, int size, const char *header)
{
    int cc;
    int n;

    if (size <= 0) {
        fprintf(stderr, "%s\n", header);
        return 0;
    }
    for (cc = 0; cc < size; cc += n) {
        if ((n = write(STDOUT_FILENO, &out[cc], size - cc)) <= 0) {
            break;
        }
    }
    if (cc < size) {
        fputs("Short write\n", stderr);
        return 0;
    }
    return cc == size;
}

/* do a command once for a specified file 'f' */
static int
rnp_cmd(rnp_t *rnp, prog_t *p, char *f)
{
    const int cleartext = 1;
    unsigned  maxsize;
    char *    out;
    char *    in;
    int       ret;
    int       cc;

    switch (p->cmd) {
    case ENCRYPT:
        if (f == NULL) {
            cc = stdin_to_mem(rnp, &in, &out, &maxsize);
            ret = rnp_encrypt_memory(
              rnp, rnp_getvar(rnp, "userid"), in, cc, out, maxsize, p->armour);
            ret = show_output(out, ret, "Bad memory encryption");
            free(in);
            free(out);
            return ret;
        }
        return rnp_encrypt_file(rnp, rnp_getvar(rnp, "userid"), f, p->output, p->armour);
    case DECRYPT:
        if (f == NULL) {
            cc = stdin_to_mem(rnp, &in, &out, &maxsize);
            ret = rnp_decrypt_memory(rnp, in, cc, out, maxsize, 0);
            ret = show_output(out, ret, "Bad memory decryption");
            free(in);
            free(out);
            return ret;
        }
        return rnp_decrypt_file(rnp, f, p->output, p->armour);
    case CLEARSIGN:
    case SIGN:
        if (f == NULL) {
            cc = stdin_to_mem(rnp, &in, &out, &maxsize);
            ret = rnp_sign_memory(rnp,
                                  rnp_getvar(rnp, "userid"),
                                  in,
                                  cc,
                                  out,
                                  maxsize,
                                  p->armour,
                                  (p->cmd == CLEARSIGN) ? cleartext : !cleartext);
            ret = show_output(out, ret, "Bad memory signature");
            free(in);
            free(out);
            return ret;
        }
        return rnp_sign_file(rnp,
                             rnp_getvar(rnp, "userid"),
                             f,
                             p->output,
                             p->armour,
                             (p->cmd == CLEARSIGN) ? cleartext : !cleartext,
                             p->detached);
    case VERIFY:
    case VERIFY_CAT:
        if (f == NULL) {
            cc = stdin_to_mem(rnp, &in, &out, &maxsize);
            ret = rnp_verify_memory(rnp,
                                    in,
                                    cc,
                                    (p->cmd == VERIFY_CAT) ? out : NULL,
                                    (p->cmd == VERIFY_CAT) ? maxsize : 0,
                                    p->armour);
            ret = show_output(out, ret, "Bad memory verification");
            free(in);
            free(out);
            return ret;
        }
        return rnp_verify_file(
          rnp, f, (p->cmd == VERIFY) ? NULL : (p->output) ? p->output : "-", p->armour);
    case LIST_PACKETS:
        if (f == NULL) {
            fprintf(stderr, "%s: No filename provided\n", __progname);
            return 0;
        }
        return rnp_list_packets(rnp, f, p->armour, NULL);
    case SHOW_KEYS:
        return rnp_validate_sigs(rnp);
    default:
        print_usage(usage);
        exit(EXIT_SUCCESS);
    }
}

/* set an option */
static int
setoption(rnp_t *rnp, prog_t *p, int val, char *arg)
{
    switch (val) {
    case COREDUMPS:
        rnp_setvar(rnp, "coredumps", "allowed");
        break;
    case ENCRYPT:
        /* for encryption, we need a userid */
        rnp_setvar(rnp, "need userid", "1");
        p->cmd = val;
        break;
    case SIGN:
    case CLEARSIGN:
        /* for signing, we need a userid and a seckey */
        rnp_setvar(rnp, "need seckey", "1");
        rnp_setvar(rnp, "need userid", "1");
        p->cmd = val;
        break;
    case DECRYPT:
        /* for decryption, we need a seckey */
        rnp_setvar(rnp, "need seckey", "1");
        p->cmd = val;
        break;
    case VERIFY:
    case VERIFY_CAT:
    case LIST_PACKETS:
    case SHOW_KEYS:
        p->cmd = val;
        break;
    case HELP_CMD:
        print_usage(usage);
        exit(EXIT_SUCCESS);
    case VERSION_CMD:
        print_praise();
        exit(EXIT_SUCCESS);
    /* options */
    case SSHKEYS:
        rnp_setvar(rnp, "keyring_format", "SSH");
        break;
    case KEYRING:
        if (arg == NULL) {
            fputs("No keyring argument provided\n", stderr);
            exit(EXIT_ERROR);
        }
        snprintf(p->keyring, sizeof(p->keyring), "%s", arg);
        break;
    case KEYRING_FORMAT:
        if (arg == NULL) {
            (void) fprintf(stderr, "No keyring format argument provided\n");
            exit(EXIT_ERROR);
        }
        rnp_setvar(rnp, "keyring_format", arg);
        break;
    case USERID:
        if (arg == NULL) {
            fputs("No userid argument provided\n", stderr);
            exit(EXIT_ERROR);
        }
        rnp_setvar(rnp, "userid", arg);
        break;
    case ARMOUR:
        p->armour = 1;
        break;
    case DETACHED:
        p->detached = 1;
        break;
    case VERBOSE:
        rnp_incvar(rnp, "verbose", 1);
        break;
    case HOMEDIR:
        if (arg == NULL) {
            (void) fprintf(stderr, "No home directory argument provided\n");
            exit(EXIT_ERROR);
        }
        rnp_set_homedir(rnp, arg, 0);
        break;
    case HASH_ALG:
        if (arg == NULL) {
            (void) fprintf(stderr, "No hash algorithm argument provided\n");
            exit(EXIT_ERROR);
        }
        rnp_setvar(rnp, "hash", arg);
        break;
    case PASSWDFD:
        if (arg == NULL) {
            (void) fprintf(stderr, "No pass-fd argument provided\n");
            exit(EXIT_ERROR);
        }
        rnp_setvar(rnp, "pass-fd", arg);
        break;
    case OUTPUT:
        if (arg == NULL) {
            (void) fprintf(stderr, "No output filename argument provided\n");
            exit(EXIT_ERROR);
        }
        if (p->output) {
            (void) free(p->output);
        }
        p->output = strdup(arg);
        break;
    case RESULTS:
        if (arg == NULL) {
            (void) fprintf(stderr, "No output filename argument provided\n");
            exit(EXIT_ERROR);
        }
        rnp_setvar(rnp, "results", arg);
        break;
    case SSHKEYFILE:
        rnp_setvar(rnp, "keyring_format", "SSH");
        rnp_setvar(rnp, "sshkeyfile", arg);
        break;
    case MAX_MEM_ALLOC:
        rnp_setvar(rnp, "max mem alloc", arg);
        break;
    case DURATION:
        rnp_setvar(rnp, "duration", arg);
        break;
    case BIRTHTIME:
        rnp_setvar(rnp, "birthtime", arg);
        break;
    case CIPHER:
        rnp_setvar(rnp, "cipher", arg);
        break;
    case NUMTRIES:
        rnp_setvar(rnp, "numtries", arg);
        break;
    case OPS_DEBUG:
        rnp_set_debug(arg);
        break;
    default:
        p->cmd = HELP_CMD;
        break;
    }
    return 1;
}

/* we have -o option=value -- parse, and process */
static int
parse_option(rnp_t *rnp, prog_t *p, const char *s)
{
    static regex_t opt;
    struct option *op;
    static int     compiled;
    regmatch_t     matches[10];
    char           option[128];
    char           value[128];

    if (!compiled) {
        compiled = 1;
        regcomp(&opt, "([^=]{1,128})(=(.*))?", REG_EXTENDED);
    }
    if (regexec(&opt, s, 10, matches, 0) == 0) {
        snprintf(option,
                 sizeof(option),
                 "%.*s",
                 (int) (matches[1].rm_eo - matches[1].rm_so),
                 &s[matches[1].rm_so]);
        if (matches[2].rm_so > 0) {
            snprintf(value,
                     sizeof(value),
                     "%.*s",
                     (int) (matches[3].rm_eo - matches[3].rm_so),
                     &s[matches[3].rm_so]);
        } else {
            value[0] = 0x0;
        }
        for (op = options; op->name; op++) {
            if (strcmp(op->name, option) == 0)
                return setoption(rnp, p, op->val, value);
        }
    }
    return 0;
}

int
main(int argc, char **argv)
{
    rnp_t  rnp;
    prog_t p;
    int    optindex;
    int    ret;
    int    ch;
    int    i;

    memset(&rnp, '\0', sizeof(rnp));
    memset(&p, '\0', sizeof(p));

    p.overwrite = 1;
    p.output = NULL;

    if (argc < 2) {
        print_usage(usage);
        exit(EXIT_ERROR);
    }

    if (!rnp_init(&rnp)) {
        fputs("fatal: cannot initialise\n", stderr);
        return EXIT_ERROR;
    }

    rnp_setvar(&rnp, "hash", DEFAULT_HASH_ALG);
    rnp_setvar(&rnp, "max mem alloc", "4194304"); /* 4MiB */

    optindex = 0;

    /* TODO: These options should be set after initialising the context. */
    while ((ch = getopt_long(argc, argv, "S:Vdeo:sv", options, &optindex)) != -1) {
        if (ch >= ENCRYPT) {
            /* getopt_long returns 0 for long options */
            if (!setoption(&rnp, &p, options[optindex].val, optarg)) {
                (void) fprintf(stderr, "Bad option\n");
            }
        } else {
            switch (ch) {
            case 'S':
                rnp_setvar(&rnp, "keyring_format", "SSH");
                rnp_setvar(&rnp, "sshkeyfile", optarg);
                break;
            case 'V':
                print_praise();
                exit(EXIT_SUCCESS);
            case 'd':
                /* for decryption, we need the seckey */
                rnp_setvar(&rnp, "need seckey", "1");
                p.cmd = DECRYPT;
                break;
            case 'e':
                /* for encryption, we need a userid */
                rnp_setvar(&rnp, "need userid", "1");
                p.cmd = ENCRYPT;
                break;
            case 'o':
                if (!parse_option(&rnp, &p, optarg)) {
                    (void) fprintf(stderr, "Bad option\n");
                }
                break;
            case 's':
                /* for signing, we need a userid and a seckey */
                rnp_setvar(&rnp, "need seckey", "1");
                rnp_setvar(&rnp, "need userid", "1");
                p.cmd = SIGN;
                break;
            case 'v':
                p.cmd = VERIFY;
                break;
            default:
                p.cmd = HELP_CMD;
                break;
            }
        }
    }

    if (!rnp_load_keys(&rnp)) {
        switch (errno) {
        case EINVAL:
            fputs("fatal: failed to load keys: bad homedir\n", stderr);
            break;
        default:
            fputs("fatal: failed to load keys\n", stderr);
        }
        return EXIT_ERROR;
    }

    /* now do the required action for each of the command line args */
    ret = EXIT_SUCCESS;
    if (optind == argc) {
        if (!rnp_cmd(&rnp, &p, NULL))
            ret = EXIT_FAILURE;
    } else {
        for (i = optind; i < argc; i++) {
            if (!rnp_cmd(&rnp, &p, argv[i])) {
                ret = EXIT_FAILURE;
            }
        }
    }
    rnp_end(&rnp);

    return ret;
}
