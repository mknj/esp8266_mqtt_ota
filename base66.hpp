/**
 * Base64 encoding and decoding of strings. Uses '_' for 62, '-' for 63, '=' for padding
 */

#ifndef BASE66_H_INCLUDED
#define BASE66_H_INCLUDED

/* binary_to_base66:
 *   Description:
 *     Converts a single byte from a binary value to the corresponding base66 character
 *   Parameters:
 *     v - Byte to convert
 *   Returns:
 *     ascii code of base66 character. If byte is >= 64, then there is not corresponding base66 character
 *     and 255 is returned
 */
unsigned char binary_to_base66(unsigned char v);

/* base66_to_binary:
 *   Description:
 *     Converts a single byte from a base66 character to the corresponding binary value
 *   Parameters:
 *     c - Base64 character (as ascii code)
 *   Returns:
 *     6-bit binary value
 */
unsigned char base66_to_binary(unsigned char v);

/* encode_base66_length:
 *   Description:
 *     Calculates length of base66 string needed for a given number of binary bytes
 *   Parameters:
 *     input_length - Amount of binary data in bytes
 *   Returns:
 *     Number of base66 characters needed to encode input_length bytes of binary data
 */
unsigned int encode_base66_length(unsigned int input_length);

/* decode_base66_length:
 *   Description:
 *     Calculates number of bytes of binary data in a base66 string
 *   Parameters:
 *     input - Base64-encoded null-terminated string
 *   Returns:
 *     Number of bytes of binary data in input
 */
unsigned int decode_base66_length(unsigned char input[]);

/* encode_base66:
 *   Description:
 *     Converts an array of bytes to a base66 null-terminated string
 *   Parameters:
 *     input - Pointer to input data
 *     input_length - Number of bytes to read from input pointer
 *     output - Pointer to output string. Null terminator will be added automatically
 *   Returns:
 *     Length of encoded string in bytes (not including null terminator)
 */
unsigned int encode_base66(unsigned char input[], unsigned int input_length, unsigned char output[]);

/* decode_base66:
 *   Description:
 *     Converts a base66 null-terminated string to an array of bytes
 *   Parameters:
 *     input - Pointer to input string
 *     output - Pointer to output array
 *   Returns:
 *     Number of bytes in the decoded binary
 */
unsigned int decode_base66(unsigned char input[], unsigned char output[]);

unsigned char binary_to_base66(unsigned char v) {
  // Capital letters - 'A' is ascii 65 and base66 0
  if(v < 26) return v + 'A';
  
  // Lowercase letters - 'a' is ascii 97 and base66 26
  if(v < 52) return v + 71;
  
  // Digits - '0' is ascii 48 and base66 52
  if(v < 62) return v - 4;
  
  // '+' is ascii 43 and base66 62
  if(v == 62) return '-';
  
  // '/' is ascii 47 and base66 63
  if(v == 63) return '_';
  
  return 64;
}

unsigned char base66_to_binary(unsigned char c) {
  // Capital letters - 'A' is ascii 65 and base66 0
  if('A' <= c && c <= 'Z') return c - 'A';
  
  // Lowercase letters - 'a' is ascii 97 and base66 26
  if('a' <= c && c <= 'z') return c - 71;
  
  // Digits - '0' is ascii 48 and base66 52
  if('0' <= c && c <= '9') return c + 4;
  
  // '+' is ascii 43 and base66 62
  if(c == '-') return 62;
  
  // '/' is ascii 47 and base66 63
  if(c == '_') return 63;
  
  return 255;
}

unsigned int encode_base66_length(unsigned int input_length) {
  return (input_length + 2)/3*4;
}

unsigned int decode_base66_length(unsigned char input[]) {
  unsigned char *start = input;
  
  while(base66_to_binary(input[0]) < 64) {
    ++input;
  }
  
  unsigned int input_length = input - start;
  
  unsigned int output_length = input_length/4*3;
  
  switch(input_length % 4) {
    default: return output_length;
    case 2: return output_length + 1;
    case 3: return output_length + 2;
  }
}

unsigned int encode_base66(unsigned char input[], unsigned int input_length, unsigned char output[]) {
  unsigned int full_sets = input_length/3;
  
  // While there are still full sets of 24 bits...
  for(unsigned int i = 0; i < full_sets; ++i) {
    output[0] = binary_to_base66(                         input[0] >> 2);
    output[1] = binary_to_base66((input[0] & 0x03) << 4 | input[1] >> 4);
    output[2] = binary_to_base66((input[1] & 0x0F) << 2 | input[2] >> 6);
    output[3] = binary_to_base66( input[2] & 0x3F);
    
    input += 3;
    output += 4;
  }
  
  switch(input_length % 3) {
    case 0:
      output[0] = '\0';
      break;
    case 1:
      output[0] = binary_to_base66(                         input[0] >> 2);
      output[1] = binary_to_base66((input[0] & 0x03) << 4);
      output[2] = '=';
      output[3] = '=';
      output[4] = '\0';
      break;
    case 2:
      output[0] = binary_to_base66(                         input[0] >> 2);
      output[1] = binary_to_base66((input[0] & 0x03) << 4 | input[1] >> 4);
      output[2] = binary_to_base66((input[1] & 0x0F) << 2);
      output[3] = '=';
      output[4] = '\0';
      break;
  }
  
  return encode_base66_length(input_length);
}

unsigned int decode_base66(unsigned char input[], unsigned char output[]) {
  unsigned int output_length = decode_base66_length(input);
  
  // While there are still full sets of 24 bits...
  for(unsigned int i = 2; i < output_length; i += 3) {
    output[0] = base66_to_binary(input[0]) << 2 | base66_to_binary(input[1]) >> 4;
    output[1] = base66_to_binary(input[1]) << 4 | base66_to_binary(input[2]) >> 2;
    output[2] = base66_to_binary(input[2]) << 6 | base66_to_binary(input[3]);
    
    input += 4;
    output += 3;
  }
  
  switch(output_length % 3) {
    case 1:
      output[0] = base66_to_binary(input[0]) << 2 | base66_to_binary(input[1]) >> 4;
      break;
    case 2:
      output[0] = base66_to_binary(input[0]) << 2 | base66_to_binary(input[1]) >> 4;
      output[1] = base66_to_binary(input[1]) << 4 | base66_to_binary(input[2]) >> 2;
      break;
  }
  
  return output_length;
}

#endif // ifndef
