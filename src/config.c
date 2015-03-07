/* -*- coding: utf-8 -*- */
/* -*- mode: c -*- */
/*
 * Dislocker -- enables to read/write on BitLocker encrypted partitions under
 * Linux
 * Copyright (C) 2012-2013  Romain Coltel, Hervé Schauer Consultants
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */


#include <getopt.h>

#include "dislocker/common.h"
#include "dislocker/config.h"



/**
 * Print program's usage
 */
void dis_usage()
{
	fprintf(stderr,
PROGNAME " by " AUTHOR ", v"VERSION " (compiled for " __OS "/" __ARCH ")\n"
"\n"
"Usage: " PROGNAME " [-hqrsv] [-l LOG_FILE] [-o OFFSET] [-V VOLUME DECRYPTMETHOD -F[N]] [-- ARGS...]\n"
"    with DECRYPTMETHOD = -p[RECOVERY_PASSWORD]|-f BEK_FILE|-u[USER_PASSWORD]|-k FVEK_FILE|-c\n"
"\n"
"Options:\n"
"    -c, --clearkey        decrypt volume using a clear key (default)\n"
"    -f, --bekfile BEKFILE\n"
"                          decrypt volume using the bek file (on USB key)\n"
"    -F, --force-block N   force use of metadata block number N (1, 2 or 3)\n"
"    -h, --help            print this help and exit\n"
"    -k, --fvek FVEK_FILE  decrypt volume using the FVEK directly\n"
"    -l, --logfile LOG_FILE\n"
"                          put messages into this file (stdout by default)\n"
"    -o, --offset OFFSET   BitLocker partition offset (default is 0)\n"
"    -p, --recovery-password[RECOVERY_PASSWORD]\n"
"                          decrypt volume using the recovery password method\n"
"    -q, --quiet           do NOT display anything\n"
"    -r, --readonly        do not allow to write on the BitLocker volume\n"
"    -s, --stateok         do not check the volume's state, assume it's ok to mount it\n"
"    -u, --user-password   decrypt volume using the user password method\n"
"    -v, --verbosity       increase verbosity (CRITICAL errors are displayed by default)\n"
"    -V, --volume VOLUME   volume to get metadata and keys from\n"
"\n"
"    --                    end of program options, beginning of FUSE's ones\n"
"\n"
"  ARGS are any arguments you want to pass to FUSE. You need to pass at least\n"
"the mount-point.\n"
"\n"
	);
}


/**
 * Hide a commandline option, replacing the actual optarg by 'X's.
 * 
 * @param opt The option to hide
 */
static void hide_opt(char* opt)
{
	if(!opt)
		return;
	
	size_t len = strlen(opt);
	
	while(len)
	{
		opt[--len] = 'X';
	}
}


/**
 * Parse arguments strings
 * 
 * @param cfg The config pointer to dis_config_t structure
 * @param argc Number of arguments given to the program
 * @param argv Arguments given to the program
 * @return Return the number of arguments which are still waiting to be studied
 */
int dis_getopts(dis_config_t* cfg, int argc, char** argv)
{
	/** See man getopt_long(3) */
	extern int optind;
	int optchar = 0;
	
	enum {
		NO_OPT,   /* No option for this argument */
		NEED_OPT, /* Need an option for this one */
		MAY_OPT   /* User may provide an option  */ 
	};
	
	/* Options which could be passed as argument */
	const char          short_opts[] = "cf:F::hk:l:o:p::qrsu::vV:";
	const struct option long_opts[] = {
		{"clearkey",          NO_OPT,   NULL, 'c'},
		{"bekfile",           NEED_OPT, NULL, 'f'},
		{"force-block",       MAY_OPT,  NULL, 'F'},
		{"help",              NO_OPT,   NULL, 'h'},
		{"fvek",              NEED_OPT, NULL, 'k'},
		{"logfile",           NEED_OPT, NULL, 'l'},
		{"offset",            NEED_OPT, NULL, 'o'},
		{"recovery-password", MAY_OPT,  NULL, 'p'},
		{"quiet",             NO_OPT,   NULL, 'q'},
		{"readonly",          NO_OPT,   NULL, 'r'},
		{"stateok",           NO_OPT,   NULL, 's'},
		{"user-password",     MAY_OPT,  NULL, 'u'},
		{"verbosity",         NO_OPT,   NULL, 'v'},
		{"volume",            NEED_OPT, NULL, 'V'},
		{0, 0, 0, 0}
	};
	
	
	while((optchar=getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1)
	{
		switch(optchar)
		{
			case 'c':
			{
				int t = TRUE;
				dis_setopt(cfg, DIS_OPT_CLEAR_KEY, &t);
				break;
			}
			case 'f':
			{
				dis_setopt(cfg, DIS_OPT_BEK_FILE_PATH, optarg);
				break;
			}
			case 'F':
			{
				off_t force;
				if(optarg)
					force = (unsigned char) strtol(optarg, NULL, 10);
				else
					force = 1;
				dis_setopt(cfg, DIS_OPT_FORCE_BLOCK, &force);
				break;
			}
			case 'h':
			{
				dis_usage();
				dis_free_args(cfg);
				exit(EXIT_SUCCESS);
			}
			case 'k':
			{
				dis_setopt(cfg, DIS_OPT_FVEK_FILE_PATH, optarg);
				break;
			}
			case 'l':
			{
				dis_setopt(cfg, DIS_OPT_LOG_FILE_PATH, optarg);
				break;
			}
			case 'o':
			{
				off_t offset = (off_t) strtoll(optarg, NULL, 10);
				dis_setopt(cfg, DIS_OPT_VOLUME_OFFSET, &offset);
				break;
			}
			case 'p':
			{
				dis_setopt(cfg, DIS_OPT_RECOVERY_PASSWORD, optarg);
				hide_opt(optarg);
				break;
			}
			case 'q':
			{
				DIS_LOGS l = L_QUIET;
				dis_setopt(cfg, DIS_OPT_VERBOSITY, &l);
				break;
			}
			case 'r':
			{
				int t = TRUE;
				dis_setopt(cfg, DIS_OPT_READ_ONLY, &t);
				break;
			}
			case 's':
			{
				int t = TRUE;
				dis_setopt(cfg, DIS_OPT_DONT_CHECK_VOLUME_STATE, &t);
				break;
			}
			case 'u':
			{
				dis_setopt(cfg, DIS_OPT_USER_PASSWORD, optarg);
				hide_opt(optarg);
				break;
			}
			case 'v':
			{
				if(cfg->verbosity != L_QUIET)
					cfg->verbosity++;
				break;
			}
			case 'V':
			{
				dis_setopt(cfg, DIS_OPT_VOLUME_PATH, optarg);
				break;
			}
			case '?':
			default:
			{
				dis_usage();
				dis_free_args(cfg);
				exit(EXIT_FAILURE);
			}
		}
	}
	
	
	/* Check verbosity */
	if(cfg->verbosity > L_DEBUG)
		cfg->verbosity = L_DEBUG;
	
	/* Check decryption method */
	if(!cfg->decryption_mean)
		cfg->decryption_mean |= DIS_USE_CLEAR_KEY;
	
	/* Check if a block is forced */
	if(cfg->force_block != 1 &&
	   cfg->force_block != 2 &&
	   cfg->force_block != 3)
		cfg->force_block = 0;
	
	
	return optind;
}


/**
 * Modify dislocker's options one-by-one
 * 
 * @param cfg Dislocker's config
 * @param opt_name The option's name to change
 * @param opt_value The new value of the option. Note that this is a pointer. If
 * NULL, the default value -which is not necessarily usable- will be set.
 */
int dis_setopt(dis_config_t* cfg, dis_opt_e opt_name, const void* opt_value)
{
	if (!cfg)
		return FALSE;
	
	
	switch(opt_name)
	{
		case DIS_OPT_VOLUME_PATH:
			if(cfg->volume_path != NULL)
				free(cfg->volume_path);
			if(opt_value == NULL)
				cfg->volume_path = NULL;
			else
			{
				cfg->volume_path = strdup((const char*) opt_value);
			}
			break;
		case DIS_OPT_CLEAR_KEY:
			if(opt_value == NULL)
				cfg->decryption_mean &= (unsigned) ~DIS_USE_CLEAR_KEY;
			else
			{
				if(*(int*) opt_value == TRUE)
					cfg->decryption_mean |= DIS_USE_CLEAR_KEY;
				else
					cfg->decryption_mean &= (unsigned) ~DIS_USE_CLEAR_KEY;
			}
			break;
		case DIS_OPT_BEK_FILE_PATH:
			if(cfg->bek_file != NULL)
				free(cfg->bek_file);
			if(opt_value == NULL)
			{
				cfg->bek_file = NULL;
				cfg->decryption_mean &= (unsigned) ~DIS_USE_BEKFILE;
			}
			else
			{
				cfg->bek_file = strdup((const char*) opt_value);
				cfg->decryption_mean |= DIS_USE_BEKFILE;
			}
			break;
		case DIS_OPT_RECOVERY_PASSWORD:
			if(cfg->recovery_password != NULL)
				free(cfg->recovery_password);
			if(opt_value == NULL)
			{
				cfg->recovery_password = NULL;
				cfg->decryption_mean &= (unsigned) ~DIS_USE_RECOVERY_PASSWORD;
			}
			else
			{
				const char* v = (const char*) opt_value;
				cfg->recovery_password = (uint8_t *) strdup(v);
				cfg->decryption_mean |= DIS_USE_RECOVERY_PASSWORD;
			}
			break;
		case DIS_OPT_USER_PASSWORD:
			if(cfg->user_password != NULL)
				free(cfg->user_password);
			if(opt_value == NULL)
			{
				cfg->user_password = NULL;
				cfg->decryption_mean &= (unsigned) ~DIS_USE_USER_PASSWORD;
			}
			else
			{
				const char* v = (const char*) opt_value;
				cfg->user_password = (uint8_t *) strdup(v);
				cfg->decryption_mean |= DIS_USE_USER_PASSWORD;
			}
			break;
		case DIS_OPT_FVEK_FILE_PATH:
			if(cfg->fvek_file != NULL)
				free(cfg->fvek_file);
			if(opt_value == NULL)
			{
				cfg->fvek_file = NULL;
				cfg->decryption_mean &= (unsigned) ~DIS_USE_FVEKFILE;
			}
			else
			{
				const char* v = (const char*) opt_value;
				cfg->fvek_file = strdup(v);
				cfg->decryption_mean |= DIS_USE_FVEKFILE;
			}
			break;
		case DIS_OPT_VERBOSITY:
			if(opt_value == NULL)
				cfg->verbosity = 0;
			else
			{
				DIS_LOGS l = *(DIS_LOGS*) opt_value;
				if(l >= L_QUIET && l <= L_DEBUG)
					cfg->verbosity = l;
				else
					cfg->verbosity = 0;
			}
			break;
		case DIS_OPT_LOG_FILE_PATH:
			if(cfg->log_file != NULL)
				free(cfg->log_file);
			if(opt_value == NULL)
				cfg->log_file = NULL;
			else
				cfg->log_file = strdup((const char*) opt_value);
			break;
		case DIS_OPT_FORCE_BLOCK:
			if(opt_value == NULL)
				cfg->force_block = 0;
			else
			{
				int uc = *(int*) opt_value;
				if(uc >= 1 && uc <= 3)
					cfg->force_block = (unsigned char) uc;
				else
					cfg->force_block = 0;
			}
			break;
		case DIS_OPT_VOLUME_OFFSET:
			if(opt_value == NULL)
				cfg->offset = 0;
			else
				cfg->offset = *(off_t*) opt_value;
			break;
		case DIS_OPT_READ_ONLY:
			if(opt_value == NULL)
				cfg->flags &= (unsigned) ~DIS_FLAG_READ_ONLY;
			else
			{
				int flag = *(int*) opt_value;
				if(flag == TRUE)
					cfg->flags |= DIS_FLAG_READ_ONLY;
				else
					cfg->flags &= (unsigned) ~DIS_FLAG_READ_ONLY;
			}
			break;
		case DIS_OPT_DONT_CHECK_VOLUME_STATE:
			if(opt_value == NULL)
				cfg->flags &= (unsigned) ~DIS_FLAG_DONT_CHECK_VOLUME_STATE;
			else
			{
				int flag = *(int*) opt_value;
				if(flag == TRUE)
					cfg->flags |= DIS_FLAG_DONT_CHECK_VOLUME_STATE;
				else
					cfg->flags &= (unsigned) ~DIS_FLAG_DONT_CHECK_VOLUME_STATE;
			}
			break;
		case DIS_OPT_INITIALIZE_STATE:
			if(opt_value == NULL)
				cfg->init_stop_at = DIS_STATE_COMPLETE_EVERYTHING;
			else
			{
				dis_state_e state = *(dis_state_e*) opt_value;
				cfg->init_stop_at = state;
			}
			break;
	}
	
	return TRUE;
}


/**
 * Free dis_config_t members
 * 
 * @param cfg Dislocker's config
 */
void dis_free_args(dis_config_t* cfg)
{
	if (!cfg)
		return;
	
	if(cfg->recovery_password)
		memclean(cfg->recovery_password,
		         strlen((char*)cfg->recovery_password) + sizeof(char));
	
	if(cfg->user_password)
		memclean(cfg->user_password,
		         strlen((char*)cfg->user_password) + sizeof(char));
	
	if(cfg->bek_file)
		memclean(cfg->bek_file, strlen(cfg->bek_file) + sizeof(char));
	
	if(cfg->fvek_file)
		memclean(cfg->fvek_file, strlen(cfg->fvek_file) + sizeof(char));
	
	if(cfg->volume_path)
		xfree(cfg->volume_path);
	
	if(cfg->log_file)
		xfree(cfg->log_file);
}


/**
 * Print read configuration
 */
void dis_print_args(dis_config_t* cfg)
{
	xprintf(L_DEBUG, "--- Config...\n");
	xprintf(L_DEBUG, "   Verbosity: %d\n", cfg->verbosity);
	xprintf(L_DEBUG, "   Trying to decrypt '%s'\n", cfg->volume_path);
	
	if(cfg->decryption_mean & DIS_USE_CLEAR_KEY)
	{
		xprintf(L_DEBUG, "   \tusing a clear key on the volume\n");
	}
	else if(cfg->decryption_mean &  DIS_USE_USER_PASSWORD)
	{
		xprintf(L_DEBUG, "   \tusing the user's password method\n");
		xprintf(L_DEBUG, "   \t\t-> '%s'\n", cfg->user_password);
	}
	else if(cfg->decryption_mean & DIS_USE_RECOVERY_PASSWORD)
	{
		xprintf(L_DEBUG, "   \tusing the recovery password method\n");
		xprintf(L_DEBUG, "   \t\t-> '%s'\n", cfg->recovery_password);
	}
	else if(cfg->decryption_mean & DIS_USE_BEKFILE)
	{
		xprintf(L_DEBUG, "   \tusing the bek file at '%s'\n", cfg->bek_file);
	}
	else if(cfg->decryption_mean & DIS_USE_FVEKFILE)
	{
		xprintf(L_DEBUG, "   \tusing the FVEK file at '%s'\n", cfg->fvek_file);
	}
	else
	{
		xprintf(L_DEBUG, "   \tnot using any decryption mean\n");
	}
	
	if(cfg->force_block)
		xprintf(
			L_DEBUG,
			"   Forced to be using metadata block n°%d\n",
			cfg->force_block
		);
	else
		xprintf(L_DEBUG, "   Using the first valid metadata block\n");
	
	if(cfg->flags & DIS_FLAG_READ_ONLY)
		xprintf(
			L_DEBUG,
			"   Not allowing any write on the BitLocker volume "
			"(read only mode)\n"
		);
	
	xprintf(L_DEBUG, "... End config ---\n");
}


/**
 * Getters for the flags
 */
int dis_is_read_only(dis_config_t* cfg)
{
	return (cfg->flags & DIS_FLAG_READ_ONLY);
}

int dis_is_volume_state_checked(dis_config_t* cfg)
{
	return !(cfg->flags & DIS_FLAG_DONT_CHECK_VOLUME_STATE);
}

