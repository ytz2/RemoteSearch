/*
 * str_search.c
 *
 *  Created by Yanhua Liu for CS820 assignment 1
 *  Created on: Sep 5, 2013
 *  Definition of str_search.h
 *  History:
 *	1 Use the strncasecmp function in <strings.h> to optimize
 *	the cmp_char_arr function backbone
 *  2 Change the declaration of last in bm_search from char to
 *  unsigned char
 */

#include"str_search.h"


/*build the shift table as defined in str_search.h
 *If the flag is set on, then insensitive shift table is built
 */
void build_shift_table(int* table, char* pattern, unsigned int flag) {
	int ind;
	int len;
	char temp_ind;
	len = strlen(pattern);
	/*
	 * initialize the table with -1
	 */
	for (ind = 0; ind < MAX_ASCII; ind++)
		table[ind] = len;

	// second round, calculate the shift

	for (ind = 0; ind < len - 1; ind++) {
		temp_ind = pattern[ind];
		if (flag != 0)
			temp_ind = tolower(temp_ind);
		table[(int) temp_ind] = len - ind - 1;
	}

}/*build_shift_table*/

/*
 * compare two char array if same return 0 else -1
 */
int cmp_char_arr(char* a, char *b, int len, unsigned int flag) {

	if (flag == 0)
		return (strncmp(a, b, len) == 0);
	else
		return (strncasecmp(a, b, len) == 0);
}/*cmp_char_arr*/

/*
 * Implement the Boyer-Moore Algorithm to check
 * if a given string a substring.
 * For simplicity, we have built the shift table
 * buffer: the object string to be searched
 * search_string: the substring
 * table: the already built shift table
 */

int boyer_moore(char* buff, char* pattern, int *table, unsigned int flag) {
	int len_buff, len_pattern, i0, shift;
	char *leading;
	unsigned char last;
	len_buff = strlen(buff);
	len_pattern = strlen(pattern);
	i0 = 0; /* align pattern with buff */
	leading = buff; /* set the leading ptr at buff[0] */
	last = buff[len_pattern - 1];
	shift = len_pattern;
	if (len_buff < len_pattern)
		/*check if it is too short to compare */
		return 0;

	while (i0 < len_buff - len_pattern + 1)
	/*loop to match*/
	{
		if (cmp_char_arr(leading, pattern, len_pattern, flag) == 1)
			return 1;
		last = leading[len_pattern - 1];
		if (flag)
			last = tolower(last);
		shift = table[(int) (last)];
		i0 += shift;
		leading += shift;
	}
	/*not found*/
	return 0;
}

/*search the search_string in buffer at begin/end
 * If FOUND return 1 else 0
 */
int search_begin(char* buffer, char* search_string, unsigned int flag) {
	/* initialize*/
	int buff_len, search_len;
	buff_len = strlen(buffer);
	search_len = strlen(search_string);
	/* check if the object str is shorter */
	if (buff_len < search_len)
		return 0;
	return cmp_char_arr(buffer, search_string, search_len, flag);
}

int search_end(char* buffer, char* search_string, unsigned int flag) {
	/* initialize */
	int buff_len, search_len;
	char* leading;
	buff_len = strlen(buffer);
	search_len = strlen(search_string);
	/* check if the object str is shorter */
	if (buff_len < search_len)
		return 0;
	leading = &(buffer[buff_len - search_len]);
	return cmp_char_arr(leading, search_string, search_len, flag);
}

/*the search_string exactly match the buffer string*/
int exact_match(char* buffer, char *search_string, unsigned int flag) {
	/* check if the str length is same */
	int buff_len, search_len;
	buff_len = strlen(buffer);
	search_len = strlen(search_string);
	if (buff_len != search_len)
		return 0;
	return cmp_char_arr(buffer, search_string, search_len, flag);
}



