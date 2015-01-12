/**
 * @file usc.c
 * @brief uSched JNI Interface
 *        uSched JNI interface - Client
 *
 * Date: 12-01-2015
 * 
 * Copyright 2014-2015 Pedro A. Hortas (pah@ucodev.org)
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
#include <jni.h>

#include "JNIUsc.h"

#include <usched/lib.h>

JNIEXPORT void JNICALL Java_JNIUsc_nativeInit(JNIEnv *env, jobject obj) {
	usched_init();
}

JNIEXPORT void JNICALL Java_JNIUsc_nativeDestroy(JNIEnv *env, jobject obj) {
	usched_destroy();
}

JNIEXPORT jstring JNICALL Java_JNIUsc_nativeTest(JNIEnv *env, jobject obj) {
	jstring result = NULL;

	result = (*env)->NewStringUTF(env, "Testing uSched interface...");

	return result;
}

JNIEXPORT jboolean JNICALL Java_JNIUsc_nativeOptSetRemoteHostname(
		JNIEnv *env,
		jobject obj,
		jstring hostname)
{
	const char *n_hostname = (*env)->GetStringUTFChars(env, hostname, 0);

	if (usched_opt_set_remote_hostname((char *) n_hostname) < 0)
		return 0;

	(*env)->ReleaseStringUTFChars(env, hostname, n_hostname);

	return 1;
}

JNIEXPORT jboolean JNICALL Java_JNIUsc_nativeOptSetRemotePort(
		JNIEnv *env,
		jobject obj,
		jstring port)
{
	const char *n_port = (*env)->GetStringUTFChars(env, port, 0);

	if (usched_opt_set_remote_port((char *) n_port) < 0)
		return 0;

	(*env)->ReleaseStringUTFChars(env, port, n_port);

	return 1;
}

JNIEXPORT jboolean JNICALL Java_JNIUsc_nativeOptSetRemoteUsername(
		JNIEnv *env,
		jobject obj,
		jstring username)
{
	const char *n_username = (*env)->GetStringUTFChars(env, username, 0);

	if (usched_opt_set_remote_username((char *) n_username) < 0)
		return 0;

	(*env)->ReleaseStringUTFChars(env, username, n_username);

	return 1;
}

JNIEXPORT jboolean JNICALL Java_JNIUsc_nativeOptSetRemotePassword(
		JNIEnv *env,
		jobject obj,
		jstring password)
{
	const char *n_password = (*env)->GetStringUTFChars(env, password, 0);

	if (usched_opt_set_remote_password((char *) n_password) < 0)
		return 0;

	(*env)->ReleaseStringUTFChars(env, password, n_password);

	return 1;
}

JNIEXPORT jboolean JNICALL Java_JNIUsc_nativeRequest(
		JNIEnv *env,
		jobject obj,
		jstring request)
{
	const char *n_request = (*env)->GetStringUTFChars(env, request, 0);

	if (usched_request((char *) n_request) < 0)
		return JNI_FALSE;

	(*env)->ReleaseStringUTFChars(env, request, n_request);

	return JNI_TRUE;
}

JNIEXPORT jlongArray JNICALL Java_JNIUsc_nativeResultGetRun(
		JNIEnv *env,
		jobject obj)
{
	uint64_t *entry_list;
	size_t nmemb;
	jlongArray result;

	usched_result_get_run(&entry_list, &nmemb);

	result = (*env)->NewLongArray(env, nmemb);

	(*env)->SetLongArrayRegion(env, result, 0, nmemb, (jlong *) entry_list);

	return result;
}

JNIEXPORT jlongArray JNICALL Java_JNIUsc_nativeResultGetStop(
		JNIEnv *env,
		jobject obj)
{
	uint64_t *entry_list;
	size_t nmemb;
	jlongArray result;

	usched_result_get_stop(&entry_list, &nmemb);

	result = (*env)->NewLongArray(env, nmemb);

	(*env)->SetLongArrayRegion(env, result, 0, nmemb, (jlong *) entry_list);

	return result;
}

JNIEXPORT jlongArray JNICALL Java_JNIUsc_nativeResultGetShow(
		JNIEnv *env,
		jobject obj)
{
	/* TODO */
	jlongArray result;

	result = (*env)->NewLongArray(env, 1);

	return result;
}

JNIEXPORT void JNICALL Java_JNIUsc_nativeResultFreeRun(
		JNIEnv *env,
		jobject obj)
{
	usched_result_free_run();
}

JNIEXPORT void JNICALL Java_JNIUsc_nativeResultFreeStop(
		JNIEnv *env,
		jobject obj)
{
	usched_result_free_stop();
}

JNIEXPORT void JNICALL Java_JNIUsc_nativeResultFreeShow(
		JNIEnv *env,
		jobject obj)
{
	usched_result_free_show();
}

JNIEXPORT jstring JNICALL Java_JNIUsc_nativeUsageErrorStr(
		JNIEnv *env,
		jobject obj,
		jint error)
{
	jstring result = NULL;

	result = (*env)->NewStringUTF(env, usched_usage_error_str((int) error));

	return result;
}

