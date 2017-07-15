/* sync on data start for feetback messages
 *
 * read from stdin to next message header
 * echo to stdout from there on
 */

#include <iostream>

static const unsigned char PROTOCOL_HDR1 = 'M';
static const unsigned char PROTOCOL_HDR2 = 'V';

int main(int argc, char** argv) {
  unsigned char hdr = 0;
  while (hdr != PROTOCOL_HDR2) {
    while (hdr != PROTOCOL_HDR1)
      std::cin >> hdr;
    std::cin >> hdr;
  }

  std::cout << PROTOCOL_HDR1 << PROTOCOL_HDR2;
  unsigned char copy;
  for (;;) {
    std::cin >> copy;
    std::cout << copy;
  }
  return 0;
}
