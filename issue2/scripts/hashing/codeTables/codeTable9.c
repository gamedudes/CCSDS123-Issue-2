#include <stdint.h>
char codeTable9[][128] = {
"00000000000","000000000010","000000000011","000000000012","00000000001X","00000000002","0000000000X","000000000100","000000000101","000000000102","00000000010X","00000000011","00000000012","0000000001X","0000000002","000000000X","000000001000","000000001001","000000001002","00000000100X","00000000101","00000000102","0000000010X","0000000011","0000000012","000000001X","0000000020","0000000021","0000000022","000000002X","00000000X","00000001","0000000200","0000000201","0000000202","000000020X","000000021","000000022","00000002X","0000000X","0000001","0000002000","0000002001","0000002002","000000200X","000000201","000000202","00000020X","00000021","00000022","0000002X","000000X","000001","000002","00000X","000010","000011","000012","00001X","00002","0000X","000100","000101","000102","00010X","00011","00012","0001X","00020","00021","00022","0002X","000X","001000","001001","001002","00100X","00101","00102","0010X","0011","0012","001X","00200","00201","00202","0020X","0021","0022","002X","00X","01","02000","02001","02002","0200X","0201","0202","020X","021","022","02X","0X","1","2","X",
};
uint32_t outPutCodes9[] = {
0x0,0x3B,0x1BF,0x3BF,0x17FFF,0x1B,0xBFF,0x07,0x07F,0x27F,0x0FFFF,0x33F,0x0BF,0x3FFF,0x13,0x3FF,0x27,0x17F,0x37F,0x1FFFF,0x19F,0x2BF,0xBFFF,0x09F,0x0DF,0x9FFF,0x33,0x2DF,0x1DF,0x07FFF,0xDFF,0x03,0x0B,0x3DF,0x03F,0x5FFF,0x01F,0x15F,0xEFFF,0x5FF,0x1D,0x2B,0x23F,0x13F,0xDFFF,0x11F,0x35F,0x1FFF,0x1EF,0x25F,0x6FFF,0x9FF,0x09,0x19,0x1FF,0x05,0x12F,0x0AF,0x77FF,0x0E,0x6FF,0x15,0x1AF,0x06F,0x0FFF,0xB7,0x10F,0x17FF,0x1E,0x08F,0x18F,0x2FFF,0x2FF,0x0D,0x16F,0x0EF,0x4FFF,0x77,0x04F,0x57FF,0x97,0x57,0x47FF,0x01,0x14F,0x0CF,0xAFFF,0xD7,0x1F7,0x27FF,0x4FF,0x6,0x11,0x1CF,0x02F,0x37FF,0x37,0x00F,0x67FF,0x17,0x0F7,0x07FF,0x0FF,0x2,0xA,0x05F,
};
uint8_t bitSizes9[] = {
2, 6, 10, 10, 17, 6, 12, 6, 10, 10, 17, 10, 10, 16, 6, 12, 6, 10, 10, 17, 9, 10, 16, 9, 10, 16, 6, 10, 10, 17, 12, 5, 6, 10, 10, 16, 9, 10, 16, 12, 5, 6, 10, 10, 16, 9, 10, 16, 9, 10, 16, 12, 5, 5, 12, 5, 9, 9, 15, 5, 11, 5, 9, 9, 15, 8, 9, 15, 5, 9, 9, 16, 11, 5, 9, 9, 15, 8, 9, 15, 8, 8, 15, 5, 9, 9, 16, 8, 9, 15, 11, 4, 5, 9, 9, 15, 8, 9, 15, 8, 9, 15, 11, 4, 4, 10, 
};