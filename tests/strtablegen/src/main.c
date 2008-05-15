/*
 * Copyright 2008 Arsen Chaloyan
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include "apt_string_table.h"
#include "apt_text_stream.h"

static apt_bool_t is_unique(const apt_str_table_item_t table[], apr_size_t count, 
							apr_size_t item_index, apr_size_t char_index, char value)
{
	size_t i;
	for(i=0; i<count; i++) {
		if(i != item_index && char_index < table[i].value.length && 
			value == table[i].value.buf[char_index]) {
			return FALSE;
		}
	}
	return TRUE;
}

static apt_bool_t string_table_key_generate(apt_str_table_item_t table[], apr_size_t count)
{
	size_t i,j;
	size_t length;
	for(i=0; i<count; i++) {
		length = table[i].value.length;
		table[i].key = length;
		for(j=0; j<length; j++) {
			if(is_unique(table,count,i,j,table[i].value.buf[j]) == TRUE) {
				table[i].key = j;
				break;
			}
		}
	}
	return TRUE;
}

#define TEST_BUFFER_SIZE 2048
static char parse_buffer[TEST_BUFFER_SIZE];

static size_t string_table_read(apt_str_table_item_t table[], apr_size_t max_count, FILE *file, apr_pool_t *pool)
{
	apt_str_table_item_t *item;
	size_t count = 0;
	apt_str_t line;
	apt_text_stream_t text_stream;

	text_stream.text.length = fread(parse_buffer, 1, sizeof(parse_buffer)-1, file);
	parse_buffer[text_stream.text.length] = '\0';
	text_stream.text.buf = parse_buffer;
	text_stream.pos = parse_buffer;

	do {
		if(apt_text_line_read(&text_stream,&line) == FALSE) {
			break;
		}
		if(line.length == 0) {
			break;
		}
		item = &table[count];
		apt_string_copy(&item->value,&line,pool);
		item->key = 0;
		count++;
	}
	while(count < max_count);

	return count;
}

static apt_bool_t string_table_write(const apt_str_table_item_t table[], apr_size_t count, FILE *file)
{
	size_t i;
	const apt_str_table_item_t *item;
	for(i=0; i<count; i++) {
		item = &table[i];
		fprintf(file,"{{\"%s\",%"APR_SIZE_T_FMT"},%"APR_SIZE_T_FMT"},\r\n",
			item->value.buf, item->value.length, item->key);
	}
	return TRUE;
}

int main(int argc, char *argv[])
{
	apr_pool_t *pool = NULL;
	apt_str_table_item_t table[100];
	size_t count;
	FILE *file_in, *file_out;

	/* one time apr global initialization */
	if(apr_initialize() != APR_SUCCESS) {
		return 0;
	}
	apr_pool_create(&pool,NULL);

	if(argc < 2) {
		printf("usage: stringtablegen stringtable.in [stringtable.out]\n");
		return 0;
	}
	file_in = fopen(argv[1], "rb");
	if(file_in == NULL) {
		printf("cannot open file %s\n", argv[1]);
		return 0;
	}

	if(argc > 2) {
		file_out = fopen(argv[2], "wb");
	}
	else {
		file_out = stdout;
	}

	/* read items (strings) from the file */
	count = string_table_read(table,100,file_in,pool);

	/* generate string table */
	string_table_key_generate(table,count);
	
	/* dump string table to the file */
	string_table_write(table,count,file_out);

	fclose(file_in);
	if(file_out != stdout) {
		fclose(file_out);
	}

	apr_pool_destroy(pool);
	/* final apr global termination */
	apr_terminate();
	return 0;
}
