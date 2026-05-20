/* =====================================================================================
MIT License

Copyright (c) 2020 Public Invention

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Note: This is a C file but is named .cpp to be loadable as an
Arduino libary! However, it is compiled (server side, for example)
as a C file.

 *
 *       Filename:  PIRDS.cpp
 *
 *    Description: Basic self-contained utilities for the PIRDS standard
 *
 *        Version:  1.1
 *        Created:  05/07/2020 10:35:51
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Robert L. Read
 *   Organization:  Public Invention
 *        License:  MIT
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <ctype.h>
#include <string.h>
#include "PIRDS.h"


#ifndef htons

#define htons(x) ( (((x)<<8)&0xFF00) | (((x)>>8)&0xFF) )
#define ntohs(x) htons(x)

#define htonl(x) ( ((x)<<24 & 0xFF000000UL) | \
                   ((x)<< 8 & 0x00FF0000UL) | \
                   ((x)>> 8 & 0x0000FF00UL) | \
                   ((x)>>24 & 0x000000FFUL) )
#define ntohl(x) htonl(x)

#endif // !defined(htons)


/* Fill the byte buffer with a PIRDS-standard bytes from the
   Measurement Object */
uint16_t fill_byte_buffer_measurement(Measurement* m,uint8_t* buff,uint16_t blim) {
  buff[0] = m->event;
  buff[1] = m->type;
  buff[2] = m->loc;
  buff[3] = m->num;
  *((uint32_t *) &buff[4]) = htonl(m->ms);
  *((int32_t *) &buff[8]) = htonl(m->val);
  return 0;
}

Measurement get_measurement_from_buffer(uint8_t* buff,uint16_t blim) {
  Measurement m;
  m.event = buff[0];
  m.type = buff[1];
  m.loc = buff[2];
  m.num = buff[3];
  m.ms = ntohl(*((uint32_t *) &buff[4])) ;
  m.val = ntohl(*((int32_t *) &buff[8])) ;
  return m;
}

uint16_t fill_JSON_buffer_measurement(Measurement* m,char* buff,uint16_t blim) {
  uint16_t rval = sprintf(buff,
  "{ \"event\": \"M\", \"type\": \"%c\", \"ms\": %lu, \"loc\": \"%c\", \"num\": %u, \"val\": %ld }",
                     m->type,
                     m->ms,
                     m->loc,
                     m->num,
                     m->val);
  return rval;
}


// From: https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
// Note: This function returns a pointer to a substring of the original string.
// If the given string was allocated dynamically, the caller must not overwrite
// that pointer with the returned value, since the original pointer must be
// deallocated using the same allocator with which it was allocated.  The return
// value must NOT be deallocated using free() etc.
char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}
// Assume double quoted, simply remove the first ant last character.
char *trimquotes(char *str)
{
  // Trim first quote...
  str++;

  // Trim trailing space
  str[strlen(str) - 1] = '\0';
  return str;
}


int assign_value_measurement(Measurement *m,char* k, char*v) {
  // First strip the key...
  char *stripped_key = trimwhitespace(k);
  char *stripped_value = trimwhitespace(v);
  if (0 == strcmp(stripped_key,"\"event\"")) {
    m->event = stripped_value[1];
  } else
  if (0 == strcmp(stripped_key,"\"type\"")) {
    m->type = stripped_value[1];
  } else
  if (0 == strcmp(stripped_key,"\"ms\"")) {
    m->ms = atoi(stripped_value);
  } else
  if (0 == strcmp(stripped_key,"\"loc\"")) {
    m->loc = stripped_value[1];
  } else
  if (0 == strcmp(stripped_key,"\"num\"")) {
    m->num = atoi(stripped_value);
  } else
  if (0 == strcmp(stripped_key,"\"val\"")) {
    m->val = atoi(stripped_value);
  } else {
    fprintf(stderr,"Key Error: %s",stripped_key);
    return 0;
  }
  return 1;
}


// Note: This is a VERY weak and specialized JSON parser
// This will fail if a string contains a colon!!!
Measurement get_measurement_from_JSON(char* buff,uint16_t blim) {
  int i = 0;
  Measurement m;
  //  char *scratch = strtok(buff, "{,:}");

  // eat the first brace...
  //  scratch = strtok(NULL, "{,:}");
  //  fprintf(stderr,"scratch %s\n",scratch);
  //  fprintf(stderr,"buff %s\n",buff);
  for(i = 0; i < 5; i++) {
    char *k = strtok((i == 0) ? buff : NULL, "{,:}");
    char *v = strtok(NULL, "{,:}");
    assign_value_measurement(&m,k,v);
  }
  char *k = strtok(NULL,"{,:}");
  char *v = strtok(NULL,"{,:}");
  assign_value_measurement(&m,k,v);
  return m;
}

uint16_t fill_byte_buffer_message(Message* m,uint8_t* buff,uint16_t blim) {
  buff[0] = m->event;
  buff[1] = m->type;
  *((uint32_t *) &buff[2]) = htonl(m->ms);
  buff[6] = m->b_size;
  for(int i = 0; i < m->b_size; i++) {
    buff[7+i] = m->buff[i];
  }
  return 0;
}

Message get_message_from_buffer(uint8_t* buff,uint16_t blim) {
  Message m;
  m.event = buff[0];
  m.type = buff[1];
  m.ms = ntohl(*((uint32_t *) &buff[2])) ;
  m.b_size = (uint8_t) buff[6];
  for(int i = 0; i < m.b_size; i++) {
    m.buff[i] = buff[i+7];
  }
  m.buff[m.b_size] = '\0';
  return m;
}

// Need to work out the limit math better...
uint16_t fill_JSON_buffer_message(Message* m,char* buff,uint16_t blim) {
  char str_buff[257];

  strcpy(str_buff,m->buff);

  // Terminate the string...
  //  int n = strlen(m->buff+6);
  //  str_buff[n] = '\0';

  //  fprintf(stderr,"str_buff %s",buff);
    int rval = sprintf(buff,
  "{ \"event\": \"E\", \"type\": \"M\", \"ms\": %lu, \"b_size\": %u, \"buff\" : \"%s\" }",
                       m->ms,
                       m->b_size,
                       str_buff);

    return rval;
}
int assign_value_message(Message *m,char* k, char*v) {
  // First strip the key...
  char *stripped_key = trimwhitespace(k);
  char *stripped_value = trimwhitespace(v);
  if (0 == strcmp(stripped_key,"\"event\"")) {
    m->event = stripped_value[1];
  } else
  if (0 == strcmp(stripped_key,"\"type\"")) {
    m->type = stripped_value[1];
  } else
  if (0 == strcmp(stripped_key,"\"ms\"")) {
    m->ms = atoi(stripped_value);
  } else
  if (0 == strcmp(stripped_key,"\"b_size\"")) {
    m->b_size = atoi(stripped_value);
  } else
  if (0 == strcmp(stripped_key,"\"buff\"")) {
    char *trimmed = trimquotes(stripped_value);
    if (strlen(trimmed) > 255) {
      // This is an internal error. We'll truncate
      memcpy(m->buff,trimmed,255);
      m->buff[255] = '\0';
    } else {
      strcpy(m->buff,trimmed);
    }
  } else {
    fprintf(stderr,"Key Error: %s",stripped_key);
    return 0;
  }
  return 1;
}

Message get_message_from_JSON(char* buff,uint16_t blim) {

  int i = 0;
  Message m;
  for(i = 0; i < 4; i++) {
    char *k = strtok((i == 0) ? buff : NULL, "{,:}");
    char *v = strtok(NULL, "{,:}");
    assign_value_message(&m,k,v);
  }
  char *k = strtok(NULL,"{,:}");
  char *v = strtok(NULL,"{,:}");
  assign_value_message(&m,k,v);
  return m;
}

char get_event_designation_char_from_json(const char* cbuff,uint16_t blim) {
  // Note: Because strtok is a destructive opearation and
  // the whole point of this operation is later use buff, we are doing
  // a copy here.
  char buff[64];
  strncpy(buff,cbuff,64);

  char *k = strtok(buff , "{,:}");
  char *v = strtok(NULL, "{,:}");
  char *stripped_key = trimwhitespace(k);
  char *stripped_value = trimwhitespace(v);
  if (0 == strcmp(stripped_key,"\"event\"")) {
    return stripped_value[1];
  } else {
    return '\0'; // same as zero, an error condition
  }

}
