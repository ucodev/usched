/**
 * @file sec.c
 * @brief uSched
 *        Security interface - Daemon
 *
 * Date: 16-08-2014
 * 
 * Copyright 2014 Pedro A. Hortas (pah@ucodev.org)
 *
 * This file is part of usched.
 *
 * usched is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * usched is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with usched.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <fsop/path.h>

#include <psec/ke.h>

#include "config.h"
#include "runtime.h"
#include "log.h"
#include "sec.h"

static int _sec_dh_keys_from_file(void) {
	int errsv = 0;
	FILE *fp = NULL;

	/* Check if a private key file exists */
	if (fsop_path_isreg(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_KEYS "/" CONFIG_USCHED_FILE_KEYS_PRIVATE) <= 0) {
		log_warn("_sec_dh_keys_from_file(): No file containing a private key was found.\n");
		return 0;
	}

	/* Check if a public key file exists */
	if (fsop_path_isreg(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_KEYS "/" CONFIG_USCHED_FILE_KEYS_PRIVATE) <= 0) {
		log_warn("_sec_dh_keys_from_file(): No file containing a public key was found.\n");
		return 0;
	}

	/* Open private key file */
	if (!(fp = fopen(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_KEYS "/" CONFIG_USCHED_FILE_KEYS_PRIVATE, "r"))) {
		errsv = errno;
		log_warn("_sec_dh_keys_from_file(): fopen(\"%s\", \"r\"): %s\n", CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_KEYS "/" CONFIG_USCHED_FILE_KEYS_PRIVATE, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read private key from file */
	if (fread(rund.sec.key_prv, sizeof(rund.sec.key_prv), 1, fp) != 1) {
		log_warn("_sec_dh_keys_from_file(): Size of private key stored in file is lesser than expected.\n");
		errno = EINVAL;
		return -1;
	}

	/* Flush and close file */
	fflush(fp);
	fclose(fp);

	/* Open public key file */
	if (!(fp = fopen(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_KEYS "/" CONFIG_USCHED_FILE_KEYS_PUBLIC, "r"))) {
		errsv = errno;
		log_warn("_sec_dh_keys_from_file(): fopen(\"%s\", \"r\"): %s\n", CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_KEYS "/" CONFIG_USCHED_FILE_KEYS_PUBLIC, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read public key from file */
	if (fread(rund.sec.key_pub, sizeof(rund.sec.key_pub), 1, fp) != 1) {
		log_warn("_sec_dh_keys_from_file(): Size of private key stored in file is lesser than expected.\n");
		errno = EINVAL;
		return -1;
	}

	/* Flush and close file */
	fflush(fp);
	fclose(fp);


	/* All good */
	return 1;
}

static int _sec_dh_keys_to_file(void) {
	int errsv = 0;
	FILE *fp = NULL;

	/* Open private key file */
	if (!(fp = fopen(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_KEYS "/" CONFIG_USCHED_FILE_KEYS_PRIVATE, "w"))) {
		errsv = errno;
		log_warn("_sec_dh_keys_to_file(): fopen(\"%s\", \"w\"): %s\n", CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_KEYS "/" CONFIG_USCHED_FILE_KEYS_PRIVATE, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read private key from file */
	if (fwrite(rund.sec.key_prv, sizeof(rund.sec.key_prv), 1, fp) != 1) {
		errsv = errno;
		log_warn("_sec_dh_keys_to_file(): Unable to write private key to file: fwrite(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Flush and close file */
	fflush(fp);
	fclose(fp);

	/* Open public key file */
	if (!(fp = fopen(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_KEYS "/" CONFIG_USCHED_FILE_KEYS_PUBLIC, "w"))) {
		errsv = errno;
		log_warn("_sec_dh_keys_to_file(): fopen(\"%s\", \"w\"): %s\n", CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_KEYS "/" CONFIG_USCHED_FILE_KEYS_PUBLIC, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read public key from file */
	if (fwrite(rund.sec.key_pub, sizeof(rund.sec.key_pub), 1, fp) != 1) {
		errsv = errno;
		log_warn("_sec_dh_keys_from_file(): Unable to write public key to file: fwrite(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Flush and close file */
	fflush(fp);
	fclose(fp);

	/* All good */
	return 0;
}

int sec_daemon_init(void) {
	int errsv = 0, ret = 0;

	/* Try to load keys from files first */
	if ((ret = _sec_dh_keys_from_file()) == 1)
		return 0;

	if (ret < 0) {
		errsv = errno;
		log_warn("sec_daemon_init(): _sec_dh_keys_from_file(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Create DH private/public key pair */
	if (sec_dh_keys_init(rund.sec.key_prv, sizeof(rund.sec.key_prv), rund.sec.key_pub, sizeof(rund.sec.key_pub)) < 0) {
		errsv = errno;
		log_warn("sec_daemon_init(): sec_dh_keys_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Serialize keys to files */
	if (_sec_dh_keys_to_file() < 0) {
		errsv = errno;
		log_warn("sec_daemon_init(): _sec_dh_keys_to_file(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int sec_daemon_compute_shared_key(unsigned char *shared_key, unsigned char *public_key) {
	int errsv = 0;

	/* Compute shared key based on remote public key*/
	if (!ke_dh_shared(shared_key, public_key, sizeof(rund.sec.key_pub), rund.sec.key_prv, sizeof(rund.sec.key_prv))) {
		errsv = errno;
		log_warn("sec_daemon_compute_shared_key(): ke_dh_shared(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

void sec_daemon_destroy(void) {
	memset(&rund.sec, 0, sizeof(rund.sec));
}

