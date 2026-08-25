#pragma once

int cmpstr(const char *str1, const char *str2) {
	const unsigned char *p1 = ( const unsigned char * )str1;
	const unsigned char *p2 = ( const unsigned char * )str2;
	
	while ( *p1 && *p1 == *p2 ) ++p1, ++p2;
	return ( *p1 > *p2 ) - ( *p2 > *p1 ) ;
}
