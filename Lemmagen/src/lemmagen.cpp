/******************************************************************************
This file is part of the lemmagen library. It gives support for lemmatization.
Copyright (C) 2011 Jernej Virag <jernej@virag.si>

The lemmagen library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
******************************************************************************/

/*
 *  This exports C interface for the lemmatizer
 */

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <lemmagen.h>
#include <RdrLemmatizer.h>

#define HAS_MUTEX __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900)

// MSVC before 14 doesn't really handle C++11
#if HAS_MUTEX
	#include <mutex>
	static std::mutex mutex_lemmatizer;
#endif

static RdrLemmatizer *lemmatizer = nullptr;


#ifdef __cplusplus
extern "C"
{
#endif

	EXPORT_API int lem_load_language_library(const char *file_name)
	{
		#if HAS_MUTEX
		std::lock_guard<std::mutex> lock(mutex_lemmatizer);
		#endif

		struct stat buf;
		// Check if file exists first
		if (stat(file_name, &buf) != 0)
		{
			return STATUS_FILE_NOT_FOUND;
		}

		if (!(buf.st_mode & S_IFREG))
		{
			return STATUS_NOT_FILE;
		}

		if (lemmatizer != nullptr)
		{
			delete lemmatizer;
		}

		try {
			lemmatizer = new RdrLemmatizer(file_name);
		} catch (...) {
			return STATUS_FILE_NOT_FOUND;
		}

		return STATUS_OK;
	}

	EXPORT_API int lem_lemmatize_word(const char *input_word, char *output_word)
	{
		// TODO figure out fast locking for this to ensure thread safety
		// Lemmatization should be reentrant, but initialization isn't.
		if (lemmatizer == nullptr)
		{
			return STATUS_FILE_NOT_LOADED;
		}

		if (output_word == nullptr) {
			return STATUS_NULL_BUFFER;
		}

		if (input_word == nullptr) {
			output_word[0] = '\0';
			return STATUS_OK;
		}

		lemmatizer->Lemmatize(input_word, output_word);
		return STATUS_OK;
	}

	EXPORT_API void lem_unload_language_library(void) {
		#if HAS_MUTEX
		std::lock_guard<std::mutex> lock(mutex_lemmatizer);
		#endif

		if (lemmatizer != nullptr) {
			delete lemmatizer;
			lemmatizer = nullptr;
		}
	}

#ifdef __cplusplus
}
#endif
