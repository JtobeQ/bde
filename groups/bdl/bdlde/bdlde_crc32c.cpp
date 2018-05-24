// bdlde_crc32c.cpp                                                   -*-C++-*-
#include <bdlde_crc32c.h>

// IMPLEMENTATION NOTES:
//
///Crc32c
///------
// Our hardware-based implementation is based on comdb2's C library 'crc32c':
//                       https://github.com/bloomberg/comdb2/tree/master/crc32c
//
///Performance
///-----------
// Below are performance comparisons of the hardware-accelerated and software
// implementations against various alternative implementations that compute a
// 32-bit CRC checksum.  They were obtained on a Linux machine with the
// following CPU architecture:
//..
//  $ lscpu
//  Architecture:          x86_64
//  CPU op-mode(s):        32-bit, 64-bit
//  Byte Order:            Little Endian
//  CPU(s):                40
//  On-line CPU(s) list:   0-39
//  Thread(s) per core:    2
//  Core(s) per socket:    10
//  Socket(s):             2
//  NUMA node(s):          2
//  Vendor ID:             GenuineIntel
//  CPU family:            6
//  Model:                 62
//  Stepping:              4
//  CPU MHz:               3001.000
//  BogoMIPS:              5982.81
//  Virtualization:        VT-x
//  L1d cache:             32K
//  L1i cache:             32K
//  L2 cache:              256K
//  L3 cache:              25600K
//  NUMA node0 CPU(s):     0-9,20-29
//  NUMA node1 CPU(s):     10-19,30-39
//..
//
///Throughput
/// - - - - -
//..
//  Default (Hardware Acceleration)| 20.363 GB per second
//  Software                       |  1.582 GB per second
//  BDE 'bdlde::crc32'             |  0.374 GB per second
//..
//
///Calculation Time
/// - - - - - - - -
// In the tables below:
//: o !Time! is an average (in absolute nanoseconds) measured over a tight loop
//:   of 100,000 iterations.
//:
//: o !Size! is the size (in bytes) of a 'char *' of random input. Note that it
//:   uses IEC base2 notation (e.g. 1Ki = 2^10 = 1024, 1Mi = 2^20 = 1,048,576).
//
///64-bit Default (Hardware Acceleration) vs. BDE's 'bdlde::crc32'
///   -   -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
//..
//  ===========================================================================
//  | Size(B) | Def time(ns) | bdlde::crc32 time(ns)| Ratio(bdlde::crc32 / Def)
//  ===========================================================================
//  |       11|             9|                    16|                     1.783
//  |       16|             9|                    28|                     3.089
//  |       21|            10|                    39|                     3.932
//  |       59|            13|                   136|                    10.323
//  |       64|            12|                   150|                    11.675
//  |       69|            13|                   161|                    11.669
//  |      251|            30|                   632|                    20.509
//  |      256|            30|                   640|                    20.660
//  |      261|            37|                   654|                    17.415
//  |     1019|           155|                  2588|                    16.698
//  |     1 Ki|            45|                  2602|                    57.324
//  |     1029|            50|                  2614|                    51.443
//  |     4091|           299|                 10436|                    34.865
//  |     4 Ki|           176|                 10448|                    59.040
//  |     4101|           190|                 10456|                    54.763
//  |    16379|           864|                 41806|                    48.366
//  |    16 Ki|           724|                 41829|                    57.721
//  |    16389|           754|                 40699|                    53.944
//  |    64 Ki|          2858|                162340|                    56.787
//  |   256 Ki|         11925|                654410|                    54.875
//  |     1 Mi|         50937|               2664898|                    52.317
//  |     4 Mi|        198662|              10562189|                    53.167
//  |    16 Mi|        796534|              42570294|                    53.444
//  |    64 Mi|       9976933|             169051561|                    16.944
//..
//
///64-bit Software (SW) vs. BDE's 'bdlde::crc32'
///   -   -  -  -  -  -  -  -  -  -  -  -  -  -
//..
//  ==========================================================================
//  | Size(B) | SW time(ns) | bdlde::crc32 time(ns) | Ratio(bdlde::crc32 / SW)
//  ==========================================================================
//  |       11|           13|                     17|                    1.229
//  |       16|           15|                     29|                    1.895
//  |       21|           26|                     39|                    1.500
//  |       59|           44|                    137|                    3.082
//  |       64|           43|                    150|                    3.428
//  |       69|           53|                    161|                    3.017
//  |      251|          158|                    629|                    3.977
//  |      256|          158|                    640|                    4.053
//  |      261|          173|                    654|                    3.777
//  |     1019|          621|                   2592|                    4.170
//  |     1 Ki|          621|                   2602|                    4.185
//  |     1029|          633|                   2614|                    4.128
//  |     4091|         2456|                  10435|                    4.248
//  |     4 Ki|         2457|                  10447|                    4.252
//  |     4101|         2464|                  10462|                    4.246
//  |    16379|         9795|                  41820|                    4.270
//  |    16 Ki|         9798|                  41838|                    4.270
//  |    16389|         9798|                  41846|                    4.271
//  |    64 Ki|        39222|                 167394|                    4.268
//  |   256 Ki|       156894|                 665589|                    4.242
//  |     1 Mi|       629828|                2656343|                    4.218
//  |     4 Mi|      2575623|               10601903|                    4.116
//  |    16 Mi|     10085862|               42775171|                    4.241
//  |    64 Mi|     40705975|              169682572|                    4.168
//..
//
///64-bit Default vs. the 'serial' CRC32-C implementation
///   -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
// Compare our hardware-based implementation to a hardware-based implementation
// for calculating CRC-32C in 64 bit mode (using SIMD - SSE 4.2) which issues
// the specialized hardware instruction in 'serial' over a loop (as opposed to
// 'triplet' -- three at a time).
//
// Both implementations ultimately leverage the same specialized hardware
// instructions for calculating CRC32-C (Intel SSE4.2 'crc32' instruction).
// However, there are differences in how fast they perform.  For sizes less
// than 1024B, the 'serial' implementation is faster.  For sizes equal to or
// larger than 1024B, the 'triplet' implementation is faster due to dividing
// the buffer into three non-overlapping parts and processing three CRC32
// calculations in parallel (see 'C(i)' in 'crc32c1024SseInt' or refer to the
// white paper linked in the implementation for 'crc32c1024SseInt').
//..
//  ========================================================================
//  | Size(B) | Default time(ns) | Serial time(ns) | Ratio(Serial / Triplet)
//  ========================================================================
//  |       11|                 9|                5|                   0.556
//  |       16|                 8|                4|                   0.500
//  |       21|                 9|                6|                   0.667
//  |       59|                12|                8|                   0.667
//  |       64|                12|                8|                   0.667
//  |       69|                13|               10|                   0.769
//  |      251|                30|               31|                   1.033
//  |      256|                32|               30|                   0.938
//  |      261|                37|               38|                   1.027
//  |     1019|               154|              153|                   0.994
//  |     1 Ki|                45|              151|                   3.356
//  |     1029|                50|              154|                   3.080
//  |     4091|               299|              634|                   2.120
//  |     4 Ki|               176|              636|                   3.614
//  |     4101|               187|              636|                   3.401
//  |    16379|               864|             2568|                   2.972
//  |    16 Ki|               724|             2567|                   3.546
//  |    16389|               751|             2572|                   3.425
//  |    64 Ki|              2947|            10296|                   3.494
//  |   256 Ki|             12275|            41235|                   3.359
//  |     1 Mi|             50918|           164927|                   3.239
//  |     4 Mi|            203426|           659671|                   3.243
//  |    16 Mi|            811794|          2638746|                   3.251
//  |    64 Mi|           7147904|         11091230|                   1.552
//..
//
///Performance (sparc)
///-------------------
// Below are software vs hardware performance comparison for different sparc
// CPUs:
//..
//  SPARC T5: 10.1465 times faster, at 710,138 iterations per second
//  SPARC T7: 10.1007 times faster, at 808,625 iterations per second
//  SPARC T8: 7.66392 times faster, at 1,013,937 iterations per second
//..

// BDE
#include <bsl_algorithm.h>
#include <bsl_cstddef.h>
#include <bsl_iostream.h>
#include <bsl_ostream.h>

#if (__cplusplus < 201103L)
#include <bslmt_once.h>
#endif

#include <bsls_assert.h>
#include <bsls_byteorderutil.h>
#include <bsls_log.h>
#include <bsls_performancehint.h>
#include <bsls_platform.h>
#include <bsls_types.h>

// Compiler-specific
#if defined(BSLS_PLATFORM_CMP_GNU) || defined(BSLS_PLATFORM_CMP_CLANG)
#include <cpuid.h>
#endif

#if defined(BSLS_PLATFORM_CPU_SPARC)
#include <sparc_crc32c.h>
#endif

namespace BloombergLP {
namespace bdlde {

namespace {

                        //=======================
                        // class Crc32cCalculator
                        //=======================

class Crc32cCalculator {
    // This class represents a singleton that detects if the current processor
    // supports hardware instructions to help calculate CRC32-C and initializes
    // a global variable with a pointer to a function that uses those
    // instructions, or provides software implementation otherwise.

    // TYPES
    typedef
    unsigned int (*Crc32cFn)(const unsigned char *data,
                             unsigned int         length,
                             unsigned int         crc);
        // Signature of the function for the calculation of CRC32-C.

    // STATIC DATA
    static Crc32cFn g_crc32cFn;
        // A global CRC32-C calculator function to compute CRC32-C checksum.

    // CREATORS
    Crc32cCalculator();
        // Create an instance of this class.

    // NOT IMPLEMENTED
    Crc32cCalculator(const Crc32cCalculator&);             // = delete;
    Crc32cCalculator& operator=(const Crc32cCalculator&);  // = delete;

  public:
    static Crc32cCalculator& instance();
        // Return a reference to the singleton object.

    // ACCESSORS
    unsigned int operator()(const unsigned char *data,
                            unsigned int         length,
                            unsigned int         crc) const;
        // Invoke the global function that calculates CRC3-C passing to this
        // function the specified 'data', 'length' and 'crc' parameters.
};

const unsigned int k_CRC_TABLE_IL8_O32[256] =
    // The following CRC lookup table was generated automagically using the
    // following model parameters:
    //
    //..
    //  Generator Polynomial = ......... 0x1EDC6F41
    //  Generator Polynomial Length = .. 32 bits
    //  Reflected Bits = ............... TRUE
    //  Table Generation Offset = ...... 32 bits
    //  Number of Slices = ............. 8 slices
    //  Slice Lengths = ................ 8 8 8 8 8 8 8 8
    //..
{
    0x00000000, 0xF26B8303, 0xE13B70F7, 0x1350F3F4,
    0xC79A971F, 0x35F1141C, 0x26A1E7E8, 0xD4CA64EB,
    0x8AD958CF, 0x78B2DBCC, 0x6BE22838, 0x9989AB3B,
    0x4D43CFD0, 0xBF284CD3, 0xAC78BF27, 0x5E133C24,
    0x105EC76F, 0xE235446C, 0xF165B798, 0x030E349B,
    0xD7C45070, 0x25AFD373, 0x36FF2087, 0xC494A384,
    0x9A879FA0, 0x68EC1CA3, 0x7BBCEF57, 0x89D76C54,
    0x5D1D08BF, 0xAF768BBC, 0xBC267848, 0x4E4DFB4B,
    0x20BD8EDE, 0xD2D60DDD, 0xC186FE29, 0x33ED7D2A,
    0xE72719C1, 0x154C9AC2, 0x061C6936, 0xF477EA35,
    0xAA64D611, 0x580F5512, 0x4B5FA6E6, 0xB93425E5,
    0x6DFE410E, 0x9F95C20D, 0x8CC531F9, 0x7EAEB2FA,
    0x30E349B1, 0xC288CAB2, 0xD1D83946, 0x23B3BA45,
    0xF779DEAE, 0x05125DAD, 0x1642AE59, 0xE4292D5A,
    0xBA3A117E, 0x4851927D, 0x5B016189, 0xA96AE28A,
    0x7DA08661, 0x8FCB0562, 0x9C9BF696, 0x6EF07595,
    0x417B1DBC, 0xB3109EBF, 0xA0406D4B, 0x522BEE48,
    0x86E18AA3, 0x748A09A0, 0x67DAFA54, 0x95B17957,
    0xCBA24573, 0x39C9C670, 0x2A993584, 0xD8F2B687,
    0x0C38D26C, 0xFE53516F, 0xED03A29B, 0x1F682198,
    0x5125DAD3, 0xA34E59D0, 0xB01EAA24, 0x42752927,
    0x96BF4DCC, 0x64D4CECF, 0x77843D3B, 0x85EFBE38,
    0xDBFC821C, 0x2997011F, 0x3AC7F2EB, 0xC8AC71E8,
    0x1C661503, 0xEE0D9600, 0xFD5D65F4, 0x0F36E6F7,
    0x61C69362, 0x93AD1061, 0x80FDE395, 0x72966096,
    0xA65C047D, 0x5437877E, 0x4767748A, 0xB50CF789,
    0xEB1FCBAD, 0x197448AE, 0x0A24BB5A, 0xF84F3859,
    0x2C855CB2, 0xDEEEDFB1, 0xCDBE2C45, 0x3FD5AF46,
    0x7198540D, 0x83F3D70E, 0x90A324FA, 0x62C8A7F9,
    0xB602C312, 0x44694011, 0x5739B3E5, 0xA55230E6,
    0xFB410CC2, 0x092A8FC1, 0x1A7A7C35, 0xE811FF36,
    0x3CDB9BDD, 0xCEB018DE, 0xDDE0EB2A, 0x2F8B6829,
    0x82F63B78, 0x709DB87B, 0x63CD4B8F, 0x91A6C88C,
    0x456CAC67, 0xB7072F64, 0xA457DC90, 0x563C5F93,
    0x082F63B7, 0xFA44E0B4, 0xE9141340, 0x1B7F9043,
    0xCFB5F4A8, 0x3DDE77AB, 0x2E8E845F, 0xDCE5075C,
    0x92A8FC17, 0x60C37F14, 0x73938CE0, 0x81F80FE3,
    0x55326B08, 0xA759E80B, 0xB4091BFF, 0x466298FC,
    0x1871A4D8, 0xEA1A27DB, 0xF94AD42F, 0x0B21572C,
    0xDFEB33C7, 0x2D80B0C4, 0x3ED04330, 0xCCBBC033,
    0xA24BB5A6, 0x502036A5, 0x4370C551, 0xB11B4652,
    0x65D122B9, 0x97BAA1BA, 0x84EA524E, 0x7681D14D,
    0x2892ED69, 0xDAF96E6A, 0xC9A99D9E, 0x3BC21E9D,
    0xEF087A76, 0x1D63F975, 0x0E330A81, 0xFC588982,
    0xB21572C9, 0x407EF1CA, 0x532E023E, 0xA145813D,
    0x758FE5D6, 0x87E466D5, 0x94B49521, 0x66DF1622,
    0x38CC2A06, 0xCAA7A905, 0xD9F75AF1, 0x2B9CD9F2,
    0xFF56BD19, 0x0D3D3E1A, 0x1E6DCDEE, 0xEC064EED,
    0xC38D26C4, 0x31E6A5C7, 0x22B65633, 0xD0DDD530,
    0x0417B1DB, 0xF67C32D8, 0xE52CC12C, 0x1747422F,
    0x49547E0B, 0xBB3FFD08, 0xA86F0EFC, 0x5A048DFF,
    0x8ECEE914, 0x7CA56A17, 0x6FF599E3, 0x9D9E1AE0,
    0xD3D3E1AB, 0x21B862A8, 0x32E8915C, 0xC083125F,
    0x144976B4, 0xE622F5B7, 0xF5720643, 0x07198540,
    0x590AB964, 0xAB613A67, 0xB831C993, 0x4A5A4A90,
    0x9E902E7B, 0x6CFBAD78, 0x7FAB5E8C, 0x8DC0DD8F,
    0xE330A81A, 0x115B2B19, 0x020BD8ED, 0xF0605BEE,
    0x24AA3F05, 0xD6C1BC06, 0xC5914FF2, 0x37FACCF1,
    0x69E9F0D5, 0x9B8273D6, 0x88D28022, 0x7AB90321,
    0xAE7367CA, 0x5C18E4C9, 0x4F48173D, 0xBD23943E,
    0xF36E6F75, 0x0105EC76, 0x12551F82, 0xE03E9C81,
    0x34F4F86A, 0xC69F7B69, 0xD5CF889D, 0x27A40B9E,
    0x79B737BA, 0x8BDCB4B9, 0x988C474D, 0x6AE7C44E,
    0xBE2DA0A5, 0x4C4623A6, 0x5F16D052, 0xAD7D5351
};

const unsigned int k_CRC_TABLE_IL8_O40[256] =
    // The following CRC lookup table was generated automagically using the
    // following model parameters:
    //
    //..
    //  Generator Polynomial = ......... 0x1EDC6F41
    //  Generator Polynomial Length = .. 32 bits
    //  Reflected Bits = ............... TRUE
    //  Table Generation Offset = ...... 32 bits
    //  Number of Slices = ............. 8 slices
    //  Slice Lengths = ................ 8 8 8 8 8 8 8 8
    //..
{
    0x00000000, 0x13A29877, 0x274530EE, 0x34E7A899,
    0x4E8A61DC, 0x5D28F9AB, 0x69CF5132, 0x7A6DC945,
    0x9D14C3B8, 0x8EB65BCF, 0xBA51F356, 0xA9F36B21,
    0xD39EA264, 0xC03C3A13, 0xF4DB928A, 0xE7790AFD,
    0x3FC5F181, 0x2C6769F6, 0x1880C16F, 0x0B225918,
    0x714F905D, 0x62ED082A, 0x560AA0B3, 0x45A838C4,
    0xA2D13239, 0xB173AA4E, 0x859402D7, 0x96369AA0,
    0xEC5B53E5, 0xFFF9CB92, 0xCB1E630B, 0xD8BCFB7C,
    0x7F8BE302, 0x6C297B75, 0x58CED3EC, 0x4B6C4B9B,
    0x310182DE, 0x22A31AA9, 0x1644B230, 0x05E62A47,
    0xE29F20BA, 0xF13DB8CD, 0xC5DA1054, 0xD6788823,
    0xAC154166, 0xBFB7D911, 0x8B507188, 0x98F2E9FF,
    0x404E1283, 0x53EC8AF4, 0x670B226D, 0x74A9BA1A,
    0x0EC4735F, 0x1D66EB28, 0x298143B1, 0x3A23DBC6,
    0xDD5AD13B, 0xCEF8494C, 0xFA1FE1D5, 0xE9BD79A2,
    0x93D0B0E7, 0x80722890, 0xB4958009, 0xA737187E,
    0xFF17C604, 0xECB55E73, 0xD852F6EA, 0xCBF06E9D,
    0xB19DA7D8, 0xA23F3FAF, 0x96D89736, 0x857A0F41,
    0x620305BC, 0x71A19DCB, 0x45463552, 0x56E4AD25,
    0x2C896460, 0x3F2BFC17, 0x0BCC548E, 0x186ECCF9,
    0xC0D23785, 0xD370AFF2, 0xE797076B, 0xF4359F1C,
    0x8E585659, 0x9DFACE2E, 0xA91D66B7, 0xBABFFEC0,
    0x5DC6F43D, 0x4E646C4A, 0x7A83C4D3, 0x69215CA4,
    0x134C95E1, 0x00EE0D96, 0x3409A50F, 0x27AB3D78,
    0x809C2506, 0x933EBD71, 0xA7D915E8, 0xB47B8D9F,
    0xCE1644DA, 0xDDB4DCAD, 0xE9537434, 0xFAF1EC43,
    0x1D88E6BE, 0x0E2A7EC9, 0x3ACDD650, 0x296F4E27,
    0x53028762, 0x40A01F15, 0x7447B78C, 0x67E52FFB,
    0xBF59D487, 0xACFB4CF0, 0x981CE469, 0x8BBE7C1E,
    0xF1D3B55B, 0xE2712D2C, 0xD69685B5, 0xC5341DC2,
    0x224D173F, 0x31EF8F48, 0x050827D1, 0x16AABFA6,
    0x6CC776E3, 0x7F65EE94, 0x4B82460D, 0x5820DE7A,
    0xFBC3FAF9, 0xE861628E, 0xDC86CA17, 0xCF245260,
    0xB5499B25, 0xA6EB0352, 0x920CABCB, 0x81AE33BC,
    0x66D73941, 0x7575A136, 0x419209AF, 0x523091D8,
    0x285D589D, 0x3BFFC0EA, 0x0F186873, 0x1CBAF004,
    0xC4060B78, 0xD7A4930F, 0xE3433B96, 0xF0E1A3E1,
    0x8A8C6AA4, 0x992EF2D3, 0xADC95A4A, 0xBE6BC23D,
    0x5912C8C0, 0x4AB050B7, 0x7E57F82E, 0x6DF56059,
    0x1798A91C, 0x043A316B, 0x30DD99F2, 0x237F0185,
    0x844819FB, 0x97EA818C, 0xA30D2915, 0xB0AFB162,
    0xCAC27827, 0xD960E050, 0xED8748C9, 0xFE25D0BE,
    0x195CDA43, 0x0AFE4234, 0x3E19EAAD, 0x2DBB72DA,
    0x57D6BB9F, 0x447423E8, 0x70938B71, 0x63311306,
    0xBB8DE87A, 0xA82F700D, 0x9CC8D894, 0x8F6A40E3,
    0xF50789A6, 0xE6A511D1, 0xD242B948, 0xC1E0213F,
    0x26992BC2, 0x353BB3B5, 0x01DC1B2C, 0x127E835B,
    0x68134A1E, 0x7BB1D269, 0x4F567AF0, 0x5CF4E287,
    0x04D43CFD, 0x1776A48A, 0x23910C13, 0x30339464,
    0x4A5E5D21, 0x59FCC556, 0x6D1B6DCF, 0x7EB9F5B8,
    0x99C0FF45, 0x8A626732, 0xBE85CFAB, 0xAD2757DC,
    0xD74A9E99, 0xC4E806EE, 0xF00FAE77, 0xE3AD3600,
    0x3B11CD7C, 0x28B3550B, 0x1C54FD92, 0x0FF665E5,
    0x759BACA0, 0x663934D7, 0x52DE9C4E, 0x417C0439,
    0xA6050EC4, 0xB5A796B3, 0x81403E2A, 0x92E2A65D,
    0xE88F6F18, 0xFB2DF76F, 0xCFCA5FF6, 0xDC68C781,
    0x7B5FDFFF, 0x68FD4788, 0x5C1AEF11, 0x4FB87766,
    0x35D5BE23, 0x26772654, 0x12908ECD, 0x013216BA,
    0xE64B1C47, 0xF5E98430, 0xC10E2CA9, 0xD2ACB4DE,
    0xA8C17D9B, 0xBB63E5EC, 0x8F844D75, 0x9C26D502,
    0x449A2E7E, 0x5738B609, 0x63DF1E90, 0x707D86E7,
    0x0A104FA2, 0x19B2D7D5, 0x2D557F4C, 0x3EF7E73B,
    0xD98EEDC6, 0xCA2C75B1, 0xFECBDD28, 0xED69455F,
    0x97048C1A, 0x84A6146D, 0xB041BCF4, 0xA3E32483
};

const unsigned int k_CRC_TABLE_IL8_O48[256] =
    // The following CRC lookup table was generated automagically using the
    // following model parameters:
    //
    //..
    //  Generator Polynomial = ......... 0x1EDC6F41
    //  Generator Polynomial Length = .. 32 bits
    //  Reflected Bits = ............... TRUE
    //  Table Generation Offset = ...... 32 bits
    //  Number of Slices = ............. 8 slices
    //  Slice Lengths = ................ 8 8 8 8 8 8 8 8
    //..
{
    0x00000000, 0xA541927E, 0x4F6F520D, 0xEA2EC073,
    0x9EDEA41A, 0x3B9F3664, 0xD1B1F617, 0x74F06469,
    0x38513EC5, 0x9D10ACBB, 0x773E6CC8, 0xD27FFEB6,
    0xA68F9ADF, 0x03CE08A1, 0xE9E0C8D2, 0x4CA15AAC,
    0x70A27D8A, 0xD5E3EFF4, 0x3FCD2F87, 0x9A8CBDF9,
    0xEE7CD990, 0x4B3D4BEE, 0xA1138B9D, 0x045219E3,
    0x48F3434F, 0xEDB2D131, 0x079C1142, 0xA2DD833C,
    0xD62DE755, 0x736C752B, 0x9942B558, 0x3C032726,
    0xE144FB14, 0x4405696A, 0xAE2BA919, 0x0B6A3B67,
    0x7F9A5F0E, 0xDADBCD70, 0x30F50D03, 0x95B49F7D,
    0xD915C5D1, 0x7C5457AF, 0x967A97DC, 0x333B05A2,
    0x47CB61CB, 0xE28AF3B5, 0x08A433C6, 0xADE5A1B8,
    0x91E6869E, 0x34A714E0, 0xDE89D493, 0x7BC846ED,
    0x0F382284, 0xAA79B0FA, 0x40577089, 0xE516E2F7,
    0xA9B7B85B, 0x0CF62A25, 0xE6D8EA56, 0x43997828,
    0x37691C41, 0x92288E3F, 0x78064E4C, 0xDD47DC32,
    0xC76580D9, 0x622412A7, 0x880AD2D4, 0x2D4B40AA,
    0x59BB24C3, 0xFCFAB6BD, 0x16D476CE, 0xB395E4B0,
    0xFF34BE1C, 0x5A752C62, 0xB05BEC11, 0x151A7E6F,
    0x61EA1A06, 0xC4AB8878, 0x2E85480B, 0x8BC4DA75,
    0xB7C7FD53, 0x12866F2D, 0xF8A8AF5E, 0x5DE93D20,
    0x29195949, 0x8C58CB37, 0x66760B44, 0xC337993A,
    0x8F96C396, 0x2AD751E8, 0xC0F9919B, 0x65B803E5,
    0x1148678C, 0xB409F5F2, 0x5E273581, 0xFB66A7FF,
    0x26217BCD, 0x8360E9B3, 0x694E29C0, 0xCC0FBBBE,
    0xB8FFDFD7, 0x1DBE4DA9, 0xF7908DDA, 0x52D11FA4,
    0x1E704508, 0xBB31D776, 0x511F1705, 0xF45E857B,
    0x80AEE112, 0x25EF736C, 0xCFC1B31F, 0x6A802161,
    0x56830647, 0xF3C29439, 0x19EC544A, 0xBCADC634,
    0xC85DA25D, 0x6D1C3023, 0x8732F050, 0x2273622E,
    0x6ED23882, 0xCB93AAFC, 0x21BD6A8F, 0x84FCF8F1,
    0xF00C9C98, 0x554D0EE6, 0xBF63CE95, 0x1A225CEB,
    0x8B277743, 0x2E66E53D, 0xC448254E, 0x6109B730,
    0x15F9D359, 0xB0B84127, 0x5A968154, 0xFFD7132A,
    0xB3764986, 0x1637DBF8, 0xFC191B8B, 0x595889F5,
    0x2DA8ED9C, 0x88E97FE2, 0x62C7BF91, 0xC7862DEF,
    0xFB850AC9, 0x5EC498B7, 0xB4EA58C4, 0x11ABCABA,
    0x655BAED3, 0xC01A3CAD, 0x2A34FCDE, 0x8F756EA0,
    0xC3D4340C, 0x6695A672, 0x8CBB6601, 0x29FAF47F,
    0x5D0A9016, 0xF84B0268, 0x1265C21B, 0xB7245065,
    0x6A638C57, 0xCF221E29, 0x250CDE5A, 0x804D4C24,
    0xF4BD284D, 0x51FCBA33, 0xBBD27A40, 0x1E93E83E,
    0x5232B292, 0xF77320EC, 0x1D5DE09F, 0xB81C72E1,
    0xCCEC1688, 0x69AD84F6, 0x83834485, 0x26C2D6FB,
    0x1AC1F1DD, 0xBF8063A3, 0x55AEA3D0, 0xF0EF31AE,
    0x841F55C7, 0x215EC7B9, 0xCB7007CA, 0x6E3195B4,
    0x2290CF18, 0x87D15D66, 0x6DFF9D15, 0xC8BE0F6B,
    0xBC4E6B02, 0x190FF97C, 0xF321390F, 0x5660AB71,
    0x4C42F79A, 0xE90365E4, 0x032DA597, 0xA66C37E9,
    0xD29C5380, 0x77DDC1FE, 0x9DF3018D, 0x38B293F3,
    0x7413C95F, 0xD1525B21, 0x3B7C9B52, 0x9E3D092C,
    0xEACD6D45, 0x4F8CFF3B, 0xA5A23F48, 0x00E3AD36,
    0x3CE08A10, 0x99A1186E, 0x738FD81D, 0xD6CE4A63,
    0xA23E2E0A, 0x077FBC74, 0xED517C07, 0x4810EE79,
    0x04B1B4D5, 0xA1F026AB, 0x4BDEE6D8, 0xEE9F74A6,
    0x9A6F10CF, 0x3F2E82B1, 0xD50042C2, 0x7041D0BC,
    0xAD060C8E, 0x08479EF0, 0xE2695E83, 0x4728CCFD,
    0x33D8A894, 0x96993AEA, 0x7CB7FA99, 0xD9F668E7,
    0x9557324B, 0x3016A035, 0xDA386046, 0x7F79F238,
    0x0B899651, 0xAEC8042F, 0x44E6C45C, 0xE1A75622,
    0xDDA47104, 0x78E5E37A, 0x92CB2309, 0x378AB177,
    0x437AD51E, 0xE63B4760, 0x0C158713, 0xA954156D,
    0xE5F54FC1, 0x40B4DDBF, 0xAA9A1DCC, 0x0FDB8FB2,
    0x7B2BEBDB, 0xDE6A79A5, 0x3444B9D6, 0x91052BA8
};

const unsigned int k_CRC_TABLE_IL8_O56[256] =
    // The following CRC lookup table was generated automagically using the
    // following model parameters:
    //
    //..
    //  Generator Polynomial = ......... 0x1EDC6F41
    //  Generator Polynomial Length = .. 32 bits
    //  Reflected Bits = ............... TRUE
    //  Table Generation Offset = ...... 32 bits
    //  Number of Slices = ............. 8 slices
    //  Slice Lengths = ................ 8 8 8 8 8 8 8 8
    //..
{
    0x00000000, 0xDD45AAB8, 0xBF672381, 0x62228939,
    0x7B2231F3, 0xA6679B4B, 0xC4451272, 0x1900B8CA,
    0xF64463E6, 0x2B01C95E, 0x49234067, 0x9466EADF,
    0x8D665215, 0x5023F8AD, 0x32017194, 0xEF44DB2C,
    0xE964B13D, 0x34211B85, 0x560392BC, 0x8B463804,
    0x924680CE, 0x4F032A76, 0x2D21A34F, 0xF06409F7,
    0x1F20D2DB, 0xC2657863, 0xA047F15A, 0x7D025BE2,
    0x6402E328, 0xB9474990, 0xDB65C0A9, 0x06206A11,
    0xD725148B, 0x0A60BE33, 0x6842370A, 0xB5079DB2,
    0xAC072578, 0x71428FC0, 0x136006F9, 0xCE25AC41,
    0x2161776D, 0xFC24DDD5, 0x9E0654EC, 0x4343FE54,
    0x5A43469E, 0x8706EC26, 0xE524651F, 0x3861CFA7,
    0x3E41A5B6, 0xE3040F0E, 0x81268637, 0x5C632C8F,
    0x45639445, 0x98263EFD, 0xFA04B7C4, 0x27411D7C,
    0xC805C650, 0x15406CE8, 0x7762E5D1, 0xAA274F69,
    0xB327F7A3, 0x6E625D1B, 0x0C40D422, 0xD1057E9A,
    0xABA65FE7, 0x76E3F55F, 0x14C17C66, 0xC984D6DE,
    0xD0846E14, 0x0DC1C4AC, 0x6FE34D95, 0xB2A6E72D,
    0x5DE23C01, 0x80A796B9, 0xE2851F80, 0x3FC0B538,
    0x26C00DF2, 0xFB85A74A, 0x99A72E73, 0x44E284CB,
    0x42C2EEDA, 0x9F874462, 0xFDA5CD5B, 0x20E067E3,
    0x39E0DF29, 0xE4A57591, 0x8687FCA8, 0x5BC25610,
    0xB4868D3C, 0x69C32784, 0x0BE1AEBD, 0xD6A40405,
    0xCFA4BCCF, 0x12E11677, 0x70C39F4E, 0xAD8635F6,
    0x7C834B6C, 0xA1C6E1D4, 0xC3E468ED, 0x1EA1C255,
    0x07A17A9F, 0xDAE4D027, 0xB8C6591E, 0x6583F3A6,
    0x8AC7288A, 0x57828232, 0x35A00B0B, 0xE8E5A1B3,
    0xF1E51979, 0x2CA0B3C1, 0x4E823AF8, 0x93C79040,
    0x95E7FA51, 0x48A250E9, 0x2A80D9D0, 0xF7C57368,
    0xEEC5CBA2, 0x3380611A, 0x51A2E823, 0x8CE7429B,
    0x63A399B7, 0xBEE6330F, 0xDCC4BA36, 0x0181108E,
    0x1881A844, 0xC5C402FC, 0xA7E68BC5, 0x7AA3217D,
    0x52A0C93F, 0x8FE56387, 0xEDC7EABE, 0x30824006,
    0x2982F8CC, 0xF4C75274, 0x96E5DB4D, 0x4BA071F5,
    0xA4E4AAD9, 0x79A10061, 0x1B838958, 0xC6C623E0,
    0xDFC69B2A, 0x02833192, 0x60A1B8AB, 0xBDE41213,
    0xBBC47802, 0x6681D2BA, 0x04A35B83, 0xD9E6F13B,
    0xC0E649F1, 0x1DA3E349, 0x7F816A70, 0xA2C4C0C8,
    0x4D801BE4, 0x90C5B15C, 0xF2E73865, 0x2FA292DD,
    0x36A22A17, 0xEBE780AF, 0x89C50996, 0x5480A32E,
    0x8585DDB4, 0x58C0770C, 0x3AE2FE35, 0xE7A7548D,
    0xFEA7EC47, 0x23E246FF, 0x41C0CFC6, 0x9C85657E,
    0x73C1BE52, 0xAE8414EA, 0xCCA69DD3, 0x11E3376B,
    0x08E38FA1, 0xD5A62519, 0xB784AC20, 0x6AC10698,
    0x6CE16C89, 0xB1A4C631, 0xD3864F08, 0x0EC3E5B0,
    0x17C35D7A, 0xCA86F7C2, 0xA8A47EFB, 0x75E1D443,
    0x9AA50F6F, 0x47E0A5D7, 0x25C22CEE, 0xF8878656,
    0xE1873E9C, 0x3CC29424, 0x5EE01D1D, 0x83A5B7A5,
    0xF90696D8, 0x24433C60, 0x4661B559, 0x9B241FE1,
    0x8224A72B, 0x5F610D93, 0x3D4384AA, 0xE0062E12,
    0x0F42F53E, 0xD2075F86, 0xB025D6BF, 0x6D607C07,
    0x7460C4CD, 0xA9256E75, 0xCB07E74C, 0x16424DF4,
    0x106227E5, 0xCD278D5D, 0xAF050464, 0x7240AEDC,
    0x6B401616, 0xB605BCAE, 0xD4273597, 0x09629F2F,
    0xE6264403, 0x3B63EEBB, 0x59416782, 0x8404CD3A,
    0x9D0475F0, 0x4041DF48, 0x22635671, 0xFF26FCC9,
    0x2E238253, 0xF36628EB, 0x9144A1D2, 0x4C010B6A,
    0x5501B3A0, 0x88441918, 0xEA669021, 0x37233A99,
    0xD867E1B5, 0x05224B0D, 0x6700C234, 0xBA45688C,
    0xA345D046, 0x7E007AFE, 0x1C22F3C7, 0xC167597F,
    0xC747336E, 0x1A0299D6, 0x782010EF, 0xA565BA57,
    0xBC65029D, 0x6120A825, 0x0302211C, 0xDE478BA4,
    0x31035088, 0xEC46FA30, 0x8E647309, 0x5321D9B1,
    0x4A21617B, 0x9764CBC3, 0xF54642FA, 0x2803E842
};

const unsigned int k_CRC_TABLE_IL8_O64[256] =
    // The following CRC lookup table was generated automagically using the
    // following model parameters:
    //
    //..
    //  Generator Polynomial = ......... 0x1EDC6F41
    //  Generator Polynomial Length = .. 32 bits
    //  Reflected Bits = ............... TRUE
    //  Table Generation Offset = ...... 32 bits
    //  Number of Slices = ............. 8 slices
    //  Slice Lengths = ................ 8 8 8 8 8 8 8 8
    //..
{
    0x00000000, 0x38116FAC, 0x7022DF58, 0x4833B0F4,
    0xE045BEB0, 0xD854D11C, 0x906761E8, 0xA8760E44,
    0xC5670B91, 0xFD76643D, 0xB545D4C9, 0x8D54BB65,
    0x2522B521, 0x1D33DA8D, 0x55006A79, 0x6D1105D5,
    0x8F2261D3, 0xB7330E7F, 0xFF00BE8B, 0xC711D127,
    0x6F67DF63, 0x5776B0CF, 0x1F45003B, 0x27546F97,
    0x4A456A42, 0x725405EE, 0x3A67B51A, 0x0276DAB6,
    0xAA00D4F2, 0x9211BB5E, 0xDA220BAA, 0xE2336406,
    0x1BA8B557, 0x23B9DAFB, 0x6B8A6A0F, 0x539B05A3,
    0xFBED0BE7, 0xC3FC644B, 0x8BCFD4BF, 0xB3DEBB13,
    0xDECFBEC6, 0xE6DED16A, 0xAEED619E, 0x96FC0E32,
    0x3E8A0076, 0x069B6FDA, 0x4EA8DF2E, 0x76B9B082,
    0x948AD484, 0xAC9BBB28, 0xE4A80BDC, 0xDCB96470,
    0x74CF6A34, 0x4CDE0598, 0x04EDB56C, 0x3CFCDAC0,
    0x51EDDF15, 0x69FCB0B9, 0x21CF004D, 0x19DE6FE1,
    0xB1A861A5, 0x89B90E09, 0xC18ABEFD, 0xF99BD151,
    0x37516AAE, 0x0F400502, 0x4773B5F6, 0x7F62DA5A,
    0xD714D41E, 0xEF05BBB2, 0xA7360B46, 0x9F2764EA,
    0xF236613F, 0xCA270E93, 0x8214BE67, 0xBA05D1CB,
    0x1273DF8F, 0x2A62B023, 0x625100D7, 0x5A406F7B,
    0xB8730B7D, 0x806264D1, 0xC851D425, 0xF040BB89,
    0x5836B5CD, 0x6027DA61, 0x28146A95, 0x10050539,
    0x7D1400EC, 0x45056F40, 0x0D36DFB4, 0x3527B018,
    0x9D51BE5C, 0xA540D1F0, 0xED736104, 0xD5620EA8,
    0x2CF9DFF9, 0x14E8B055, 0x5CDB00A1, 0x64CA6F0D,
    0xCCBC6149, 0xF4AD0EE5, 0xBC9EBE11, 0x848FD1BD,
    0xE99ED468, 0xD18FBBC4, 0x99BC0B30, 0xA1AD649C,
    0x09DB6AD8, 0x31CA0574, 0x79F9B580, 0x41E8DA2C,
    0xA3DBBE2A, 0x9BCAD186, 0xD3F96172, 0xEBE80EDE,
    0x439E009A, 0x7B8F6F36, 0x33BCDFC2, 0x0BADB06E,
    0x66BCB5BB, 0x5EADDA17, 0x169E6AE3, 0x2E8F054F,
    0x86F90B0B, 0xBEE864A7, 0xF6DBD453, 0xCECABBFF,
    0x6EA2D55C, 0x56B3BAF0, 0x1E800A04, 0x269165A8,
    0x8EE76BEC, 0xB6F60440, 0xFEC5B4B4, 0xC6D4DB18,
    0xABC5DECD, 0x93D4B161, 0xDBE70195, 0xE3F66E39,
    0x4B80607D, 0x73910FD1, 0x3BA2BF25, 0x03B3D089,
    0xE180B48F, 0xD991DB23, 0x91A26BD7, 0xA9B3047B,
    0x01C50A3F, 0x39D46593, 0x71E7D567, 0x49F6BACB,
    0x24E7BF1E, 0x1CF6D0B2, 0x54C56046, 0x6CD40FEA,
    0xC4A201AE, 0xFCB36E02, 0xB480DEF6, 0x8C91B15A,
    0x750A600B, 0x4D1B0FA7, 0x0528BF53, 0x3D39D0FF,
    0x954FDEBB, 0xAD5EB117, 0xE56D01E3, 0xDD7C6E4F,
    0xB06D6B9A, 0x887C0436, 0xC04FB4C2, 0xF85EDB6E,
    0x5028D52A, 0x6839BA86, 0x200A0A72, 0x181B65DE,
    0xFA2801D8, 0xC2396E74, 0x8A0ADE80, 0xB21BB12C,
    0x1A6DBF68, 0x227CD0C4, 0x6A4F6030, 0x525E0F9C,
    0x3F4F0A49, 0x075E65E5, 0x4F6DD511, 0x777CBABD,
    0xDF0AB4F9, 0xE71BDB55, 0xAF286BA1, 0x9739040D,
    0x59F3BFF2, 0x61E2D05E, 0x29D160AA, 0x11C00F06,
    0xB9B60142, 0x81A76EEE, 0xC994DE1A, 0xF185B1B6,
    0x9C94B463, 0xA485DBCF, 0xECB66B3B, 0xD4A70497,
    0x7CD10AD3, 0x44C0657F, 0x0CF3D58B, 0x34E2BA27,
    0xD6D1DE21, 0xEEC0B18D, 0xA6F30179, 0x9EE26ED5,
    0x36946091, 0x0E850F3D, 0x46B6BFC9, 0x7EA7D065,
    0x13B6D5B0, 0x2BA7BA1C, 0x63940AE8, 0x5B856544,
    0xF3F36B00, 0xCBE204AC, 0x83D1B458, 0xBBC0DBF4,
    0x425B0AA5, 0x7A4A6509, 0x3279D5FD, 0x0A68BA51,
    0xA21EB415, 0x9A0FDBB9, 0xD23C6B4D, 0xEA2D04E1,
    0x873C0134, 0xBF2D6E98, 0xF71EDE6C, 0xCF0FB1C0,
    0x6779BF84, 0x5F68D028, 0x175B60DC, 0x2F4A0F70,
    0xCD796B76, 0xF56804DA, 0xBD5BB42E, 0x854ADB82,
    0x2D3CD5C6, 0x152DBA6A, 0x5D1E0A9E, 0x650F6532,
    0x081E60E7, 0x300F0F4B, 0x783CBFBF, 0x402DD013,
    0xE85BDE57, 0xD04AB1FB, 0x9879010F, 0xA0686EA3
};

const unsigned int k_CRC_TABLE_IL8_O72[256] =
    // The following CRC lookup table was generated automagically using the
    // following model parameters:
    //
    //..
    //  Generator Polynomial = ......... 0x1EDC6F41
    //  Generator Polynomial Length = .. 32 bits
    //  Reflected Bits = ............... TRUE
    //  Table Generation Offset = ...... 32 bits
    //  Number of Slices = ............. 8 slices
    //  Slice Lengths = ................ 8 8 8 8 8 8 8 8
    //..
{
    0x00000000, 0xEF306B19, 0xDB8CA0C3, 0x34BCCBDA,
    0xB2F53777, 0x5DC55C6E, 0x697997B4, 0x8649FCAD,
    0x6006181F, 0x8F367306, 0xBB8AB8DC, 0x54BAD3C5,
    0xD2F32F68, 0x3DC34471, 0x097F8FAB, 0xE64FE4B2,
    0xC00C303E, 0x2F3C5B27, 0x1B8090FD, 0xF4B0FBE4,
    0x72F90749, 0x9DC96C50, 0xA975A78A, 0x4645CC93,
    0xA00A2821, 0x4F3A4338, 0x7B8688E2, 0x94B6E3FB,
    0x12FF1F56, 0xFDCF744F, 0xC973BF95, 0x2643D48C,
    0x85F4168D, 0x6AC47D94, 0x5E78B64E, 0xB148DD57,
    0x370121FA, 0xD8314AE3, 0xEC8D8139, 0x03BDEA20,
    0xE5F20E92, 0x0AC2658B, 0x3E7EAE51, 0xD14EC548,
    0x570739E5, 0xB83752FC, 0x8C8B9926, 0x63BBF23F,
    0x45F826B3, 0xAAC84DAA, 0x9E748670, 0x7144ED69,
    0xF70D11C4, 0x183D7ADD, 0x2C81B107, 0xC3B1DA1E,
    0x25FE3EAC, 0xCACE55B5, 0xFE729E6F, 0x1142F576,
    0x970B09DB, 0x783B62C2, 0x4C87A918, 0xA3B7C201,
    0x0E045BEB, 0xE13430F2, 0xD588FB28, 0x3AB89031,
    0xBCF16C9C, 0x53C10785, 0x677DCC5F, 0x884DA746,
    0x6E0243F4, 0x813228ED, 0xB58EE337, 0x5ABE882E,
    0xDCF77483, 0x33C71F9A, 0x077BD440, 0xE84BBF59,
    0xCE086BD5, 0x213800CC, 0x1584CB16, 0xFAB4A00F,
    0x7CFD5CA2, 0x93CD37BB, 0xA771FC61, 0x48419778,
    0xAE0E73CA, 0x413E18D3, 0x7582D309, 0x9AB2B810,
    0x1CFB44BD, 0xF3CB2FA4, 0xC777E47E, 0x28478F67,
    0x8BF04D66, 0x64C0267F, 0x507CEDA5, 0xBF4C86BC,
    0x39057A11, 0xD6351108, 0xE289DAD2, 0x0DB9B1CB,
    0xEBF65579, 0x04C63E60, 0x307AF5BA, 0xDF4A9EA3,
    0x5903620E, 0xB6330917, 0x828FC2CD, 0x6DBFA9D4,
    0x4BFC7D58, 0xA4CC1641, 0x9070DD9B, 0x7F40B682,
    0xF9094A2F, 0x16392136, 0x2285EAEC, 0xCDB581F5,
    0x2BFA6547, 0xC4CA0E5E, 0xF076C584, 0x1F46AE9D,
    0x990F5230, 0x763F3929, 0x4283F2F3, 0xADB399EA,
    0x1C08B7D6, 0xF338DCCF, 0xC7841715, 0x28B47C0C,
    0xAEFD80A1, 0x41CDEBB8, 0x75712062, 0x9A414B7B,
    0x7C0EAFC9, 0x933EC4D0, 0xA7820F0A, 0x48B26413,
    0xCEFB98BE, 0x21CBF3A7, 0x1577387D, 0xFA475364,
    0xDC0487E8, 0x3334ECF1, 0x0788272B, 0xE8B84C32,
    0x6EF1B09F, 0x81C1DB86, 0xB57D105C, 0x5A4D7B45,
    0xBC029FF7, 0x5332F4EE, 0x678E3F34, 0x88BE542D,
    0x0EF7A880, 0xE1C7C399, 0xD57B0843, 0x3A4B635A,
    0x99FCA15B, 0x76CCCA42, 0x42700198, 0xAD406A81,
    0x2B09962C, 0xC439FD35, 0xF08536EF, 0x1FB55DF6,
    0xF9FAB944, 0x16CAD25D, 0x22761987, 0xCD46729E,
    0x4B0F8E33, 0xA43FE52A, 0x90832EF0, 0x7FB345E9,
    0x59F09165, 0xB6C0FA7C, 0x827C31A6, 0x6D4C5ABF,
    0xEB05A612, 0x0435CD0B, 0x308906D1, 0xDFB96DC8,
    0x39F6897A, 0xD6C6E263, 0xE27A29B9, 0x0D4A42A0,
    0x8B03BE0D, 0x6433D514, 0x508F1ECE, 0xBFBF75D7,
    0x120CEC3D, 0xFD3C8724, 0xC9804CFE, 0x26B027E7,
    0xA0F9DB4A, 0x4FC9B053, 0x7B757B89, 0x94451090,
    0x720AF422, 0x9D3A9F3B, 0xA98654E1, 0x46B63FF8,
    0xC0FFC355, 0x2FCFA84C, 0x1B736396, 0xF443088F,
    0xD200DC03, 0x3D30B71A, 0x098C7CC0, 0xE6BC17D9,
    0x60F5EB74, 0x8FC5806D, 0xBB794BB7, 0x544920AE,
    0xB206C41C, 0x5D36AF05, 0x698A64DF, 0x86BA0FC6,
    0x00F3F36B, 0xEFC39872, 0xDB7F53A8, 0x344F38B1,
    0x97F8FAB0, 0x78C891A9, 0x4C745A73, 0xA344316A,
    0x250DCDC7, 0xCA3DA6DE, 0xFE816D04, 0x11B1061D,
    0xF7FEE2AF, 0x18CE89B6, 0x2C72426C, 0xC3422975,
    0x450BD5D8, 0xAA3BBEC1, 0x9E87751B, 0x71B71E02,
    0x57F4CA8E, 0xB8C4A197, 0x8C786A4D, 0x63480154,
    0xE501FDF9, 0x0A3196E0, 0x3E8D5D3A, 0xD1BD3623,
    0x37F2D291, 0xD8C2B988, 0xEC7E7252, 0x034E194B,
    0x8507E5E6, 0x6A378EFF, 0x5E8B4525, 0xB1BB2E3C
};

const unsigned int k_CRC_TABLE_IL8_O80[256] =
    // The following CRC lookup table was generated automagically using the
    // following model parameters:
    //
    //..
    //  Generator Polynomial = ......... 0x1EDC6F41
    //  Generator Polynomial Length = .. 32 bits
    //  Reflected Bits = ............... TRUE
    //  Table Generation Offset = ...... 32 bits
    //  Number of Slices = ............. 8 slices
    //  Slice Lengths = ................ 8 8 8 8 8 8 8 8
    //..
{
    0x00000000, 0x68032CC8, 0xD0065990, 0xB8057558,
    0xA5E0C5D1, 0xCDE3E919, 0x75E69C41, 0x1DE5B089,
    0x4E2DFD53, 0x262ED19B, 0x9E2BA4C3, 0xF628880B,
    0xEBCD3882, 0x83CE144A, 0x3BCB6112, 0x53C84DDA,
    0x9C5BFAA6, 0xF458D66E, 0x4C5DA336, 0x245E8FFE,
    0x39BB3F77, 0x51B813BF, 0xE9BD66E7, 0x81BE4A2F,
    0xD27607F5, 0xBA752B3D, 0x02705E65, 0x6A7372AD,
    0x7796C224, 0x1F95EEEC, 0xA7909BB4, 0xCF93B77C,
    0x3D5B83BD, 0x5558AF75, 0xED5DDA2D, 0x855EF6E5,
    0x98BB466C, 0xF0B86AA4, 0x48BD1FFC, 0x20BE3334,
    0x73767EEE, 0x1B755226, 0xA370277E, 0xCB730BB6,
    0xD696BB3F, 0xBE9597F7, 0x0690E2AF, 0x6E93CE67,
    0xA100791B, 0xC90355D3, 0x7106208B, 0x19050C43,
    0x04E0BCCA, 0x6CE39002, 0xD4E6E55A, 0xBCE5C992,
    0xEF2D8448, 0x872EA880, 0x3F2BDDD8, 0x5728F110,
    0x4ACD4199, 0x22CE6D51, 0x9ACB1809, 0xF2C834C1,
    0x7AB7077A, 0x12B42BB2, 0xAAB15EEA, 0xC2B27222,
    0xDF57C2AB, 0xB754EE63, 0x0F519B3B, 0x6752B7F3,
    0x349AFA29, 0x5C99D6E1, 0xE49CA3B9, 0x8C9F8F71,
    0x917A3FF8, 0xF9791330, 0x417C6668, 0x297F4AA0,
    0xE6ECFDDC, 0x8EEFD114, 0x36EAA44C, 0x5EE98884,
    0x430C380D, 0x2B0F14C5, 0x930A619D, 0xFB094D55,
    0xA8C1008F, 0xC0C22C47, 0x78C7591F, 0x10C475D7,
    0x0D21C55E, 0x6522E996, 0xDD279CCE, 0xB524B006,
    0x47EC84C7, 0x2FEFA80F, 0x97EADD57, 0xFFE9F19F,
    0xE20C4116, 0x8A0F6DDE, 0x320A1886, 0x5A09344E,
    0x09C17994, 0x61C2555C, 0xD9C72004, 0xB1C40CCC,
    0xAC21BC45, 0xC422908D, 0x7C27E5D5, 0x1424C91D,
    0xDBB77E61, 0xB3B452A9, 0x0BB127F1, 0x63B20B39,
    0x7E57BBB0, 0x16549778, 0xAE51E220, 0xC652CEE8,
    0x959A8332, 0xFD99AFFA, 0x459CDAA2, 0x2D9FF66A,
    0x307A46E3, 0x58796A2B, 0xE07C1F73, 0x887F33BB,
    0xF56E0EF4, 0x9D6D223C, 0x25685764, 0x4D6B7BAC,
    0x508ECB25, 0x388DE7ED, 0x808892B5, 0xE88BBE7D,
    0xBB43F3A7, 0xD340DF6F, 0x6B45AA37, 0x034686FF,
    0x1EA33676, 0x76A01ABE, 0xCEA56FE6, 0xA6A6432E,
    0x6935F452, 0x0136D89A, 0xB933ADC2, 0xD130810A,
    0xCCD53183, 0xA4D61D4B, 0x1CD36813, 0x74D044DB,
    0x27180901, 0x4F1B25C9, 0xF71E5091, 0x9F1D7C59,
    0x82F8CCD0, 0xEAFBE018, 0x52FE9540, 0x3AFDB988,
    0xC8358D49, 0xA036A181, 0x1833D4D9, 0x7030F811,
    0x6DD54898, 0x05D66450, 0xBDD31108, 0xD5D03DC0,
    0x8618701A, 0xEE1B5CD2, 0x561E298A, 0x3E1D0542,
    0x23F8B5CB, 0x4BFB9903, 0xF3FEEC5B, 0x9BFDC093,
    0x546E77EF, 0x3C6D5B27, 0x84682E7F, 0xEC6B02B7,
    0xF18EB23E, 0x998D9EF6, 0x2188EBAE, 0x498BC766,
    0x1A438ABC, 0x7240A674, 0xCA45D32C, 0xA246FFE4,
    0xBFA34F6D, 0xD7A063A5, 0x6FA516FD, 0x07A63A35,
    0x8FD9098E, 0xE7DA2546, 0x5FDF501E, 0x37DC7CD6,
    0x2A39CC5F, 0x423AE097, 0xFA3F95CF, 0x923CB907,
    0xC1F4F4DD, 0xA9F7D815, 0x11F2AD4D, 0x79F18185,
    0x6414310C, 0x0C171DC4, 0xB412689C, 0xDC114454,
    0x1382F328, 0x7B81DFE0, 0xC384AAB8, 0xAB878670,
    0xB66236F9, 0xDE611A31, 0x66646F69, 0x0E6743A1,
    0x5DAF0E7B, 0x35AC22B3, 0x8DA957EB, 0xE5AA7B23,
    0xF84FCBAA, 0x904CE762, 0x2849923A, 0x404ABEF2,
    0xB2828A33, 0xDA81A6FB, 0x6284D3A3, 0x0A87FF6B,
    0x17624FE2, 0x7F61632A, 0xC7641672, 0xAF673ABA,
    0xFCAF7760, 0x94AC5BA8, 0x2CA92EF0, 0x44AA0238,
    0x594FB2B1, 0x314C9E79, 0x8949EB21, 0xE14AC7E9,
    0x2ED97095, 0x46DA5C5D, 0xFEDF2905, 0x96DC05CD,
    0x8B39B544, 0xE33A998C, 0x5B3FECD4, 0x333CC01C,
    0x60F48DC6, 0x08F7A10E, 0xB0F2D456, 0xD8F1F89E,
    0xC5144817, 0xAD1764DF, 0x15121187, 0x7D113D4F
};

const unsigned int k_CRC_TABLE_IL8_O88[256] =
    // The following CRC lookup table was generated automagically using the
    // following model parameters:
    //
    //..
    //  Generator Polynomial = ......... 0x1EDC6F41
    //  Generator Polynomial Length = .. 32 bits
    //  Reflected Bits = ............... TRUE
    //  Table Generation Offset = ...... 32 bits
    //  Number of Slices = ............. 8 slices
    //  Slice Lengths = ................ 8 8 8 8 8 8 8 8
    //..
{
    0x00000000, 0x493C7D27, 0x9278FA4E, 0xDB448769,
    0x211D826D, 0x6821FF4A, 0xB3657823, 0xFA590504,
    0x423B04DA, 0x0B0779FD, 0xD043FE94, 0x997F83B3,
    0x632686B7, 0x2A1AFB90, 0xF15E7CF9, 0xB86201DE,
    0x847609B4, 0xCD4A7493, 0x160EF3FA, 0x5F328EDD,
    0xA56B8BD9, 0xEC57F6FE, 0x37137197, 0x7E2F0CB0,
    0xC64D0D6E, 0x8F717049, 0x5435F720, 0x1D098A07,
    0xE7508F03, 0xAE6CF224, 0x7528754D, 0x3C14086A,
    0x0D006599, 0x443C18BE, 0x9F789FD7, 0xD644E2F0,
    0x2C1DE7F4, 0x65219AD3, 0xBE651DBA, 0xF759609D,
    0x4F3B6143, 0x06071C64, 0xDD439B0D, 0x947FE62A,
    0x6E26E32E, 0x271A9E09, 0xFC5E1960, 0xB5626447,
    0x89766C2D, 0xC04A110A, 0x1B0E9663, 0x5232EB44,
    0xA86BEE40, 0xE1579367, 0x3A13140E, 0x732F6929,
    0xCB4D68F7, 0x827115D0, 0x593592B9, 0x1009EF9E,
    0xEA50EA9A, 0xA36C97BD, 0x782810D4, 0x31146DF3,
    0x1A00CB32, 0x533CB615, 0x8878317C, 0xC1444C5B,
    0x3B1D495F, 0x72213478, 0xA965B311, 0xE059CE36,
    0x583BCFE8, 0x1107B2CF, 0xCA4335A6, 0x837F4881,
    0x79264D85, 0x301A30A2, 0xEB5EB7CB, 0xA262CAEC,
    0x9E76C286, 0xD74ABFA1, 0x0C0E38C8, 0x453245EF,
    0xBF6B40EB, 0xF6573DCC, 0x2D13BAA5, 0x642FC782,
    0xDC4DC65C, 0x9571BB7B, 0x4E353C12, 0x07094135,
    0xFD504431, 0xB46C3916, 0x6F28BE7F, 0x2614C358,
    0x1700AEAB, 0x5E3CD38C, 0x857854E5, 0xCC4429C2,
    0x361D2CC6, 0x7F2151E1, 0xA465D688, 0xED59ABAF,
    0x553BAA71, 0x1C07D756, 0xC743503F, 0x8E7F2D18,
    0x7426281C, 0x3D1A553B, 0xE65ED252, 0xAF62AF75,
    0x9376A71F, 0xDA4ADA38, 0x010E5D51, 0x48322076,
    0xB26B2572, 0xFB575855, 0x2013DF3C, 0x692FA21B,
    0xD14DA3C5, 0x9871DEE2, 0x4335598B, 0x0A0924AC,
    0xF05021A8, 0xB96C5C8F, 0x6228DBE6, 0x2B14A6C1,
    0x34019664, 0x7D3DEB43, 0xA6796C2A, 0xEF45110D,
    0x151C1409, 0x5C20692E, 0x8764EE47, 0xCE589360,
    0x763A92BE, 0x3F06EF99, 0xE44268F0, 0xAD7E15D7,
    0x572710D3, 0x1E1B6DF4, 0xC55FEA9D, 0x8C6397BA,
    0xB0779FD0, 0xF94BE2F7, 0x220F659E, 0x6B3318B9,
    0x916A1DBD, 0xD856609A, 0x0312E7F3, 0x4A2E9AD4,
    0xF24C9B0A, 0xBB70E62D, 0x60346144, 0x29081C63,
    0xD3511967, 0x9A6D6440, 0x4129E329, 0x08159E0E,
    0x3901F3FD, 0x703D8EDA, 0xAB7909B3, 0xE2457494,
    0x181C7190, 0x51200CB7, 0x8A648BDE, 0xC358F6F9,
    0x7B3AF727, 0x32068A00, 0xE9420D69, 0xA07E704E,
    0x5A27754A, 0x131B086D, 0xC85F8F04, 0x8163F223,
    0xBD77FA49, 0xF44B876E, 0x2F0F0007, 0x66337D20,
    0x9C6A7824, 0xD5560503, 0x0E12826A, 0x472EFF4D,
    0xFF4CFE93, 0xB67083B4, 0x6D3404DD, 0x240879FA,
    0xDE517CFE, 0x976D01D9, 0x4C2986B0, 0x0515FB97,
    0x2E015D56, 0x673D2071, 0xBC79A718, 0xF545DA3F,
    0x0F1CDF3B, 0x4620A21C, 0x9D642575, 0xD4585852,
    0x6C3A598C, 0x250624AB, 0xFE42A3C2, 0xB77EDEE5,
    0x4D27DBE1, 0x041BA6C6, 0xDF5F21AF, 0x96635C88,
    0xAA7754E2, 0xE34B29C5, 0x380FAEAC, 0x7133D38B,
    0x8B6AD68F, 0xC256ABA8, 0x19122CC1, 0x502E51E6,
    0xE84C5038, 0xA1702D1F, 0x7A34AA76, 0x3308D751,
    0xC951D255, 0x806DAF72, 0x5B29281B, 0x1215553C,
    0x230138CF, 0x6A3D45E8, 0xB179C281, 0xF845BFA6,
    0x021CBAA2, 0x4B20C785, 0x906440EC, 0xD9583DCB,
    0x613A3C15, 0x28064132, 0xF342C65B, 0xBA7EBB7C,
    0x4027BE78, 0x091BC35F, 0xD25F4436, 0x9B633911,
    0xA777317B, 0xEE4B4C5C, 0x350FCB35, 0x7C33B612,
    0x866AB316, 0xCF56CE31, 0x14124958, 0x5D2E347F,
    0xE54C35A1, 0xAC704886, 0x7734CFEF, 0x3E08B2C8,
    0xC451B7CC, 0x8D6DCAEB, 0x56294D82, 0x1F1530A5
};

#if defined(BSLS_PLATFORM_CPU_SPARC)
static
unsigned int
sparcHardware(const unsigned char *data,
              unsigned int         length,
              unsigned int         crc)
{
    const unsigned int result = sparc_crc32c(const_cast<unsigned char*>(data),
                                             length,
                                             crc ^ 0xFFFFFFFF);
    // sparc returns value in BigEndian, but in order to be cross-platform, we
    // always use LittleEndian representation of the CRC32C value, therefore
    // swap the endianess of the result.
    return bsls::ByteOrderUtil::swapBytes(result);
}
#endif  // BSLS_PLATFORM_CPU_SPARC


static
unsigned int
crc32cSoftware(const unsigned char *data,
               unsigned int         length,
               unsigned int         crc)
    // Return the CRC32-C value calculated in software for the specified 'data'
    // over the specified 'length' number of bytes, using the specified 'crc'
    // value as the starting point for the calculation.
{
    crc = crc ^ ~0U; // INIT = 0xFFFFFFFF: Initial value of the register

    // [1] Process bytes one at a time until we reach the alignment boundary
    //     (e.g. 8-byte)
    bsls::Types::IntPtr misaligned =
                                    reinterpret_cast<bsls::Types::IntPtr>(data)
                                  & (sizeof(bsls::Types::IntPtr) - 1);

    unsigned adj = misaligned
        ? static_cast<unsigned>(sizeof(bsls::Types::IntPtr) - misaligned)
        : 0;
        // No. of bytes from 'data' to next alignment boundary
    if (adj > length) {
        // the specified length to process is less than the distance from
        // 'data' to the next alignment boundary.
        adj = length;
    }

    int i = 0;
    switch (adj) { // distance from 'data' to next alignment boundary <= 7
      case 7: {
        crc = k_CRC_TABLE_IL8_O32[(crc ^ data[i]) & 0x000000FF] ^ (crc >> 8);
        ++i;
      }  // NO BREAK
      case 6: {
        crc = k_CRC_TABLE_IL8_O32[(crc ^ data[i]) & 0x000000FF] ^ (crc >> 8);
        ++i;
      }  // NO BREAK
      case 5: {
        crc = k_CRC_TABLE_IL8_O32[(crc ^ data[i]) & 0x000000FF] ^ (crc >> 8);
        ++i;
      }  // NO BREAK
      case 4: {
        crc = k_CRC_TABLE_IL8_O32[(crc ^ data[i]) & 0x000000FF] ^ (crc >> 8);
        ++i;
      }  // NO BREAK
      case 3: {
        crc = k_CRC_TABLE_IL8_O32[(crc ^ data[i]) & 0x000000FF] ^ (crc >> 8);
        ++i;
      }  // NO BREAK
      case 2: {
        crc = k_CRC_TABLE_IL8_O32[(crc ^ data[i]) & 0x000000FF] ^ (crc >> 8);
        ++i;
      }  // NO BREAK
      case 1: {
        crc = k_CRC_TABLE_IL8_O32[(crc ^ data[i]) & 0x000000FF] ^ (crc >> 8);
        ++i;
      }  // NO BREAK
    }
    length -= adj;
    data   += i;

    // [2] Process 8 bytes at a time until we have fewer than 8 bytes left.
    const unsigned char *end = data + (length & (~0x7));
        // end of last 8-byte block
    while (data < end) {
        // read two little endian integers
        unsigned int u32a, u32b;
        u32a  =   (data[0] << 0)
                | (data[1] << 8)
                | (data[2] << 16)
                | (data[3] << 24);
        data += 4;

        u32b  =   (data[0] << 0)
                | (data[1] << 8)
                | (data[2] << 16)
                | (data[3] << 24);
        data += 4;

        crc ^= u32a;

        unsigned int term1 =   k_CRC_TABLE_IL8_O88[crc & 0x000000FF]
                             ^ k_CRC_TABLE_IL8_O80[(crc >> 8) & 0x000000FF];
        unsigned int term2 = crc >> 16;

        crc   =   term1
                ^ k_CRC_TABLE_IL8_O72[term2 & 0x000000FF]
                ^ k_CRC_TABLE_IL8_O64[(term2 >> 8) & 0x000000FF];
        term1 =   k_CRC_TABLE_IL8_O56[u32b & 0x000000FF]
                ^ k_CRC_TABLE_IL8_O48[(u32b >> 8) & 0x000000FF];
        term2 = u32b >> 16;
        crc   =   crc
                ^ term1
                ^ k_CRC_TABLE_IL8_O40[term2  & 0x000000FF]
                ^ k_CRC_TABLE_IL8_O32[(term2 >> 8) & 0x000000FF];
    }

    // [3] Process the last 7 (or less) bytes remaining after the last
    //     alignment boundary
    length &= 0x7;
    i       = 0;
    switch (length) {
      case 7: {
        crc = k_CRC_TABLE_IL8_O32[(crc ^ data[i]) & 0x000000FF] ^ (crc >> 8);
        ++i;
      }  // NO BREAK
      case 6: {
        crc = k_CRC_TABLE_IL8_O32[(crc ^ data[i]) & 0x000000FF] ^ (crc >> 8);
        ++i;
      }  // NO BREAK
      case 5: {
        crc = k_CRC_TABLE_IL8_O32[(crc ^ data[i]) & 0x000000FF] ^ (crc >> 8);
        ++i;
      }  // NO BREAK
      case 4: {
        crc = k_CRC_TABLE_IL8_O32[(crc ^ data[i]) & 0x000000FF] ^ (crc >> 8);
        ++i;
      }  // NO BREAK
      case 3: {
        crc = k_CRC_TABLE_IL8_O32[(crc ^ data[i]) & 0x000000FF] ^ (crc >> 8);
        ++i;
      }  // NO BREAK
      case 2: {
        crc = k_CRC_TABLE_IL8_O32[(crc ^ data[i]) & 0x000000FF] ^ (crc >> 8);
        ++i;
      }  // NO BREAK
      case 1: {
        crc = k_CRC_TABLE_IL8_O32[(crc ^ data[i]) & 0x000000FF] ^ (crc >> 8);
        ++i;
      }  // NO BREAK
    }

    return crc ^ ~0U; // XOROUT = true: Do a final XOR on output
}


#if defined(BSLS_PLATFORM_CMP_GNU) || defined(BSLS_PLATFORM_CMP_CLANG)

#  ifdef BSLS_PLATFORM_CPU_64_BIT

                              // ---------------
                              // Helper Routines
                              // ---------------

static
unsigned int crc32c8s(const unsigned char *data,
                      unsigned int         length,
                      unsigned int         crc);
    // Calculate the CRC32-C value (calculated in slices of 8 bytes) using SSE
    // intrinsic for the specified 'data' over the specified 'length' number
    // of bytes, using the specified 'crc' value as the starting point for the
    // calculation.

static
unsigned int crc32cUntilAligned(const unsigned char **dataPtr,
                                unsigned int         *lengthPtr,
                                unsigned int          crc);
    // Calculate CRC32-C for the bytes between the buffer pointed to by the
    // specified 'dataPtr' and either the first alignment boundary (e.g.,
    // 8-byte boundary) therafter or the end of the buffer, whichever is
    // closest.  The buffer pointed to by 'dataPtr' will advance to the nearest
    // alignment boundary or the end of the buffer, whichever is first.  The
    // end of the buffer is determined by the specified 'lengthPtr', and the
    // length pointed to by 'lengthPtr' will be decremented (between 0-7 times)
    // to account for the number of bytes that were processed.  The specified
    // 'crc' value is used as the starting point for the calculation.

static
unsigned int crc32c1024SseInt(const unsigned char *data,
                              unsigned int         crc);
    // Calculate the CRC32-C value (using SSE intrinsics) for the specified
    // 'data' over exactly 1024 bytes, using the specified 'crc' value as the
    // starting point for the calculation.  Behavior is undefined unless the
    // buffer pointed to by 'data' contains at least 1024 bytes.  Note that
    // 'data' need not be at an alignment boundary.

static
unsigned int crc32c8s(const unsigned char *data,
                      unsigned int         length,
                      unsigned int         crc)
{
    const unsigned char * const end = data + length;

    // Process bytes one at a time until we reach an 8-byte boundary (or until
    // we reach the end of the buffer, whichever is sooner).
    crc = crc32cUntilAligned(&data, &length, crc);

    // Process 8 bytes at a time doing aligned 64-bit reads
    const bsls::Types::Uint64 *dataUint64 =
                           reinterpret_cast<const bsls::Types::Uint64 *>(data);
    const bsls::Types::Uint64 *endUint64 = dataUint64 + (length / 8);

    while (dataUint64 < endUint64) {
        crc = static_cast<unsigned int>(__builtin_ia32_crc32di(crc,
                                                               *dataUint64));
        ++dataUint64;
    }

    // Process the last 7 (or less) bytes remaining after the last alignment
    // boundary
    data = reinterpret_cast<const unsigned char *>(endUint64);

    const bsls::Types::IntPtr diff = end - data;
    int                       i    = 0;
    switch(diff) {
      case 7: {
        crc = __builtin_ia32_crc32qi(crc, data[i]);
        ++i;
      }  // NO BREAK
      case 6: {
        crc = __builtin_ia32_crc32qi(crc, data[i]);
        ++i;
      }  // NO BREAK
      case 5: {
        crc = __builtin_ia32_crc32qi(crc, data[i]);
        ++i;
      }  // NO BREAK
      case 4: {
        crc = __builtin_ia32_crc32qi(crc, data[i]);
        ++i;
      }  // NO BREAK
      case 3: {
        crc = __builtin_ia32_crc32qi(crc, data[i]);
        ++i;
      }  // NO BREAK
      case 2: {
        crc = __builtin_ia32_crc32qi(crc, data[i]);
        ++i;
      }  // NO BREAK
      case 1: {
        crc = __builtin_ia32_crc32qi(crc, data[i]);
        ++i;
      }  // NO BREAK
    }

    return crc;
}

// Compute CRC32-C for 1024 bytes using SSE & recombine using lookup tables
const unsigned int k_MUL_TABLE1_336[256] =
{
    0x00000000, 0x8F158014, 0x1BC776D9, 0x94D2F6CD,
    0x378EEDB2, 0xB89B6DA6, 0x2C499B6B, 0xA35C1B7F,
    0x6F1DDB64, 0xE0085B70, 0x74DAADBD, 0xFBCF2DA9,
    0x589336D6, 0xD786B6C2, 0x4354400F, 0xCC41C01B,
    0xDE3BB6C8, 0x512E36DC, 0xC5FCC011, 0x4AE94005,
    0xE9B55B7A, 0x66A0DB6E, 0xF2722DA3, 0x7D67ADB7,
    0xB1266DAC, 0x3E33EDB8, 0xAAE11B75, 0x25F49B61,
    0x86A8801E, 0x09BD000A, 0x9D6FF6C7, 0x127A76D3,
    0xB99B1B61, 0x368E9B75, 0xA25C6DB8, 0x2D49EDAC,
    0x8E15F6D3, 0x010076C7, 0x95D2800A, 0x1AC7001E,
    0xD686C005, 0x59934011, 0xCD41B6DC, 0x425436C8,
    0xE1082DB7, 0x6E1DADA3, 0xFACF5B6E, 0x75DADB7A,
    0x67A0ADA9, 0xE8B52DBD, 0x7C67DB70, 0xF3725B64,
    0x502E401B, 0xDF3BC00F, 0x4BE936C2, 0xC4FCB6D6,
    0x08BD76CD, 0x87A8F6D9, 0x137A0014, 0x9C6F8000,
    0x3F339B7F, 0xB0261B6B, 0x24F4EDA6, 0xABE16DB2,
    0x76DA4033, 0xF9CFC027, 0x6D1D36EA, 0xE208B6FE,
    0x4154AD81, 0xCE412D95, 0x5A93DB58, 0xD5865B4C,
    0x19C79B57, 0x96D21B43, 0x0200ED8E, 0x8D156D9A,
    0x2E4976E5, 0xA15CF6F1, 0x358E003C, 0xBA9B8028,
    0xA8E1F6FB, 0x27F476EF, 0xB3268022, 0x3C330036,
    0x9F6F1B49, 0x107A9B5D, 0x84A86D90, 0x0BBDED84,
    0xC7FC2D9F, 0x48E9AD8B, 0xDC3B5B46, 0x532EDB52,
    0xF072C02D, 0x7F674039, 0xEBB5B6F4, 0x64A036E0,
    0xCF415B52, 0x4054DB46, 0xD4862D8B, 0x5B93AD9F,
    0xF8CFB6E0, 0x77DA36F4, 0xE308C039, 0x6C1D402D,
    0xA05C8036, 0x2F490022, 0xBB9BF6EF, 0x348E76FB,
    0x97D26D84, 0x18C7ED90, 0x8C151B5D, 0x03009B49,
    0x117AED9A, 0x9E6F6D8E, 0x0ABD9B43, 0x85A81B57,
    0x26F40028, 0xA9E1803C, 0x3D3376F1, 0xB226F6E5,
    0x7E6736FE, 0xF172B6EA, 0x65A04027, 0xEAB5C033,
    0x49E9DB4C, 0xC6FC5B58, 0x522EAD95, 0xDD3B2D81,
    0xEDB48066, 0x62A10072, 0xF673F6BF, 0x796676AB,
    0xDA3A6DD4, 0x552FEDC0, 0xC1FD1B0D, 0x4EE89B19,
    0x82A95B02, 0x0DBCDB16, 0x996E2DDB, 0x167BADCF,
    0xB527B6B0, 0x3A3236A4, 0xAEE0C069, 0x21F5407D,
    0x338F36AE, 0xBC9AB6BA, 0x28484077, 0xA75DC063,
    0x0401DB1C, 0x8B145B08, 0x1FC6ADC5, 0x90D32DD1,
    0x5C92EDCA, 0xD3876DDE, 0x47559B13, 0xC8401B07,
    0x6B1C0078, 0xE409806C, 0x70DB76A1, 0xFFCEF6B5,
    0x542F9B07, 0xDB3A1B13, 0x4FE8EDDE, 0xC0FD6DCA,
    0x63A176B5, 0xECB4F6A1, 0x7866006C, 0xF7738078,
    0x3B324063, 0xB427C077, 0x20F536BA, 0xAFE0B6AE,
    0x0CBCADD1, 0x83A92DC5, 0x177BDB08, 0x986E5B1C,
    0x8A142DCF, 0x0501ADDB, 0x91D35B16, 0x1EC6DB02,
    0xBD9AC07D, 0x328F4069, 0xA65DB6A4, 0x294836B0,
    0xE509F6AB, 0x6A1C76BF, 0xFECE8072, 0x71DB0066,
    0xD2871B19, 0x5D929B0D, 0xC9406DC0, 0x4655EDD4,
    0x9B6EC055, 0x147B4041, 0x80A9B68C, 0x0FBC3698,
    0xACE02DE7, 0x23F5ADF3, 0xB7275B3E, 0x3832DB2A,
    0xF4731B31, 0x7B669B25, 0xEFB46DE8, 0x60A1EDFC,
    0xC3FDF683, 0x4CE87697, 0xD83A805A, 0x572F004E,
    0x4555769D, 0xCA40F689, 0x5E920044, 0xD1878050,
    0x72DB9B2F, 0xFDCE1B3B, 0x691CEDF6, 0xE6096DE2,
    0x2A48ADF9, 0xA55D2DED, 0x318FDB20, 0xBE9A5B34,
    0x1DC6404B, 0x92D3C05F, 0x06013692, 0x8914B686,
    0x22F5DB34, 0xADE05B20, 0x3932ADED, 0xB6272DF9,
    0x157B3686, 0x9A6EB692, 0x0EBC405F, 0x81A9C04B,
    0x4DE80050, 0xC2FD8044, 0x562F7689, 0xD93AF69D,
    0x7A66EDE2, 0xF5736DF6, 0x61A19B3B, 0xEEB41B2F,
    0xFCCE6DFC, 0x73DBEDE8, 0xE7091B25, 0x681C9B31,
    0xCB40804E, 0x4455005A, 0xD087F697, 0x5F927683,
    0x93D3B698, 0x1CC6368C, 0x8814C041, 0x07014055,
    0xA45D5B2A, 0x2B48DB3E, 0xBF9A2DF3, 0x308FADE7
};

const unsigned int k_MUL_TABLE1_672[256] =
{
    0x00000000, 0xE417F38A, 0xCDC391E5, 0x29D4626F,
    0x9E6B553B, 0x7A7CA6B1, 0x53A8C4DE, 0xB7BF3754,
    0x393ADC87, 0xDD2D2F0D, 0xF4F94D62, 0x10EEBEE8,
    0xA75189BC, 0x43467A36, 0x6A921859, 0x8E85EBD3,
    0x7275B90E, 0x96624A84, 0xBFB628EB, 0x5BA1DB61,
    0xEC1EEC35, 0x08091FBF, 0x21DD7DD0, 0xC5CA8E5A,
    0x4B4F6589, 0xAF589603, 0x868CF46C, 0x629B07E6,
    0xD52430B2, 0x3133C338, 0x18E7A157, 0xFCF052DD,
    0xE4EB721C, 0x00FC8196, 0x2928E3F9, 0xCD3F1073,
    0x7A802727, 0x9E97D4AD, 0xB743B6C2, 0x53544548,
    0xDDD1AE9B, 0x39C65D11, 0x10123F7E, 0xF405CCF4,
    0x43BAFBA0, 0xA7AD082A, 0x8E796A45, 0x6A6E99CF,
    0x969ECB12, 0x72893898, 0x5B5D5AF7, 0xBF4AA97D,
    0x08F59E29, 0xECE26DA3, 0xC5360FCC, 0x2121FC46,
    0xAFA41795, 0x4BB3E41F, 0x62678670, 0x867075FA,
    0x31CF42AE, 0xD5D8B124, 0xFC0CD34B, 0x181B20C1,
    0xCC3A92C9, 0x282D6143, 0x01F9032C, 0xE5EEF0A6,
    0x5251C7F2, 0xB6463478, 0x9F925617, 0x7B85A59D,
    0xF5004E4E, 0x1117BDC4, 0x38C3DFAB, 0xDCD42C21,
    0x6B6B1B75, 0x8F7CE8FF, 0xA6A88A90, 0x42BF791A,
    0xBE4F2BC7, 0x5A58D84D, 0x738CBA22, 0x979B49A8,
    0x20247EFC, 0xC4338D76, 0xEDE7EF19, 0x09F01C93,
    0x8775F740, 0x636204CA, 0x4AB666A5, 0xAEA1952F,
    0x191EA27B, 0xFD0951F1, 0xD4DD339E, 0x30CAC014,
    0x28D1E0D5, 0xCCC6135F, 0xE5127130, 0x010582BA,
    0xB6BAB5EE, 0x52AD4664, 0x7B79240B, 0x9F6ED781,
    0x11EB3C52, 0xF5FCCFD8, 0xDC28ADB7, 0x383F5E3D,
    0x8F806969, 0x6B979AE3, 0x4243F88C, 0xA6540B06,
    0x5AA459DB, 0xBEB3AA51, 0x9767C83E, 0x73703BB4,
    0xC4CF0CE0, 0x20D8FF6A, 0x090C9D05, 0xED1B6E8F,
    0x639E855C, 0x878976D6, 0xAE5D14B9, 0x4A4AE733,
    0xFDF5D067, 0x19E223ED, 0x30364182, 0xD421B208,
    0x9D995363, 0x798EA0E9, 0x505AC286, 0xB44D310C,
    0x03F20658, 0xE7E5F5D2, 0xCE3197BD, 0x2A266437,
    0xA4A38FE4, 0x40B47C6E, 0x69601E01, 0x8D77ED8B,
    0x3AC8DADF, 0xDEDF2955, 0xF70B4B3A, 0x131CB8B0,
    0xEFECEA6D, 0x0BFB19E7, 0x222F7B88, 0xC6388802,
    0x7187BF56, 0x95904CDC, 0xBC442EB3, 0x5853DD39,
    0xD6D636EA, 0x32C1C560, 0x1B15A70F, 0xFF025485,
    0x48BD63D1, 0xACAA905B, 0x857EF234, 0x616901BE,
    0x7972217F, 0x9D65D2F5, 0xB4B1B09A, 0x50A64310,
    0xE7197444, 0x030E87CE, 0x2ADAE5A1, 0xCECD162B,
    0x4048FDF8, 0xA45F0E72, 0x8D8B6C1D, 0x699C9F97,
    0xDE23A8C3, 0x3A345B49, 0x13E03926, 0xF7F7CAAC,
    0x0B079871, 0xEF106BFB, 0xC6C40994, 0x22D3FA1E,
    0x956CCD4A, 0x717B3EC0, 0x58AF5CAF, 0xBCB8AF25,
    0x323D44F6, 0xD62AB77C, 0xFFFED513, 0x1BE92699,
    0xAC5611CD, 0x4841E247, 0x61958028, 0x858273A2,
    0x51A3C1AA, 0xB5B43220, 0x9C60504F, 0x7877A3C5,
    0xCFC89491, 0x2BDF671B, 0x020B0574, 0xE61CF6FE,
    0x68991D2D, 0x8C8EEEA7, 0xA55A8CC8, 0x414D7F42,
    0xF6F24816, 0x12E5BB9C, 0x3B31D9F3, 0xDF262A79,
    0x23D678A4, 0xC7C18B2E, 0xEE15E941, 0x0A021ACB,
    0xBDBD2D9F, 0x59AADE15, 0x707EBC7A, 0x94694FF0,
    0x1AECA423, 0xFEFB57A9, 0xD72F35C6, 0x3338C64C,
    0x8487F118, 0x60900292, 0x494460FD, 0xAD539377,
    0xB548B3B6, 0x515F403C, 0x788B2253, 0x9C9CD1D9,
    0x2B23E68D, 0xCF341507, 0xE6E07768, 0x02F784E2,
    0x8C726F31, 0x68659CBB, 0x41B1FED4, 0xA5A60D5E,
    0x12193A0A, 0xF60EC980, 0xDFDAABEF, 0x3BCD5865,
    0xC73D0AB8, 0x232AF932, 0x0AFE9B5D, 0xEEE968D7,
    0x59565F83, 0xBD41AC09, 0x9495CE66, 0x70823DEC,
    0xFE07D63F, 0x1A1025B5, 0x33C447DA, 0xD7D3B450,
    0x606C8304, 0x847B708E, 0xADAF12E1, 0x49B8E16B
};

static
inline
unsigned int crc32cSse64bit(const unsigned char *data,
                            unsigned int         length,
                            unsigned int         crc)
    // Calculate the CRC32-C value (using SSE intrinsics) for the specified
    // 'data' over the specified 'length' number of bytes, using the specified
    // 'crc' value as the starting point for the calculation.  Processing is 8
    // or 1024 bytes at a time, depending on 'length', and lookup tables are
    // used for recombination.
{
    crc = crc ^ ~0U; // INIT = 0xFFFFFFFF: Initial value of the register

    // Process ('length' mod 1024) bytes
    unsigned int i = length % 1024;
    if (i) {
        length -= i; // length = k * 1024 for k in {0, 1, 2, ...}
        crc     = crc32c8s(data, i, crc);
        data   += i;
        i       = 0;
    }

    // Process remaining 'k' chunks of 1024 bytes each
    while (i < length) {
        crc  = crc32c1024SseInt(&data[i], crc);
        i   += 1024;
    }

    return crc ^ ~0U; // XOROUT = true: Do a final XOR on output
}

static
unsigned int crc32cUntilAligned(const unsigned char **dataPtr,
                                unsigned int         *lengthPtr,
                                unsigned int          crc)
{
    const unsigned char *data       = *dataPtr;
    unsigned int         length     = *lengthPtr;
    bsls::Types::IntPtr  misaligned =
                                    reinterpret_cast<bsls::Types::IntPtr>(data)
                                  & (sizeof(bsls::Types::IntPtr) - 1);

    unsigned adj = misaligned
        ? static_cast<unsigned>(sizeof(bsls::Types::IntPtr) - misaligned)
        : 0;
        // No. of bytes from 'data' to next alignment boundary
    if (adj > length) {
        // the specified length to process is less than the distance from
        // 'data' to the next alignment boundary.
        adj = length;
    }

    int i = 0;
    switch (adj) { // distance from 'data' to next alignment boundary <= 7
      case 7: crc = __builtin_ia32_crc32qi(crc, data[i]); ++i;
      case 6: crc = __builtin_ia32_crc32qi(crc, data[i]); ++i;
      case 5: crc = __builtin_ia32_crc32qi(crc, data[i]); ++i;
      case 4: crc = __builtin_ia32_crc32qi(crc, data[i]); ++i;
      case 3: crc = __builtin_ia32_crc32qi(crc, data[i]); ++i;
      case 2: crc = __builtin_ia32_crc32qi(crc, data[i]); ++i;
      case 1: crc = __builtin_ia32_crc32qi(crc, data[i]); ++i;
    }

     length    -= adj;
    *lengthPtr  = length;
    *dataPtr    = data + i;

    return crc;
}

static
unsigned int crc32c1024SseInt(const unsigned char *data,
                              unsigned int         crc)
{
// Intel White Paper: Fast CRC Computation for iSCSI Polynomial Using CRC32
// Instruction
// http://www.intel.com/content/dam/www/public/us/en/documents/white-papers/
//                             crc-iscsi-polynomial-crc32-instruction-paper.pdf
#define C(i)                                \
    c1 = __builtin_ia32_crc32di(c1, b1[i]); \
    c2 = __builtin_ia32_crc32di(c2, b2[i]); \
    c3 = __builtin_ia32_crc32di(c3, b3[i]); \

    bsls::Types::Uint64 c1, c2, c3, tmp;

    const bsls::Types::Uint64 *b8 =
                           reinterpret_cast<const bsls::Types::Uint64 *>(data);
    const bsls::Types::Uint64 *b1 = &b8[1];
    const bsls::Types::Uint64 *b2 = &b8[43];
    const bsls::Types::Uint64 *b3 = &b8[85];

    c2 = c3 = 0;
    c1 = __builtin_ia32_crc32di(crc, b8[0]);

    C(0);  C(1);  C(2);  C(3);  C(4);  C(5);  C(6);  C(7);  C(8);  C(9);
    C(10); C(11); C(12); C(13); C(14); C(15); C(16); C(17); C(18); C(19);
    C(20); C(21); C(22); C(23); C(24); C(25); C(26); C(27); C(28); C(29);
    C(30); C(31); C(32); C(33); C(34); C(35); C(36); C(37); C(38); C(39);
    C(40); C(41);

    // merge in c2
    tmp  = b8[127];
    tmp ^= k_MUL_TABLE1_336[c2 & 0xFF];
    tmp ^= static_cast<bsls::Types::Uint64>(
                                    k_MUL_TABLE1_336[(c2 >> 8) & 0xFF])  << 8;
    tmp ^= static_cast<bsls::Types::Uint64>(
                                    k_MUL_TABLE1_336[(c2 >> 16) & 0xFF]) << 16;
    tmp ^= static_cast<bsls::Types::Uint64>(
                                    k_MUL_TABLE1_336[(c2 >> 24) & 0xFF]) << 24;

    // merge in c1
    tmp ^= k_MUL_TABLE1_672[c1 & 0xFF];
    tmp ^= static_cast<bsls::Types::Uint64>(
                                    k_MUL_TABLE1_672[(c1 >> 8) & 0xFF])  << 8;
    tmp ^= static_cast<bsls::Types::Uint64>(
                                    k_MUL_TABLE1_672[(c1 >> 16) & 0xFF]) << 16;
    tmp ^= static_cast<bsls::Types::Uint64>(
                                    k_MUL_TABLE1_672[(c1 >> 24) & 0xFF]) << 24;

    return static_cast<unsigned int>(__builtin_ia32_crc32di(c3, tmp));
#undef C
}

#  endif // BSLS_PLATFORM_CPU_64_BIT

static
unsigned int crc32cHardwareSerial(const unsigned char *data,
                                  unsigned int         length,
                                  unsigned int         crc)
    // Calculate the CRC32-C value (using SSE intrinsic) for the specified
    // 'data' over the specified 'length' number of bytes, using the specified
    // 'crc' value as the starting point for the calculation.  Unlike
    // 'crc32cSse64bit', this function does not leverage instruction level
    // parallelism, but rather calculates consecutive CRCs in "serial".
{
    crc = crc ^ ~0U; // INIT = 0xFFFFFFFF: Initial value of the register

#  ifdef BSLS_PLATFORM_CPU_64_BIT
    // 64 bit implementation
    unsigned int  sum    = crc;
    unsigned int  offset = 0;

    // Process bytes one at a time until we reach an 8-byte boundary and can
    // start doing aligned 64-bit reads.
    bsls::Types::Uint64 mask =   reinterpret_cast<bsls::Types::Uint64>(data)
                               & (sizeof(bsls::Types::Uint64) - 1);
    if (mask != 0) {
        bsls::Types::Uint64 limit =
                        bsl::min(static_cast<bsls::Types::Uint64>(length),
                                 sizeof(bsls::Types::Uint64) - mask);
        while (offset < limit) {
            sum = __builtin_ia32_crc32qi(sum, data[offset]);
            offset++;
        }
    }

    // Process 8 bytes at a time until we have fewer than 8 bytes left.
    while (offset + sizeof(bsls::Types::Uint64) <= length) {
        const bsls::Types::Uint64 *src =
                reinterpret_cast<const bsls::Types::Uint64 *>(data + offset);
        sum     = static_cast<unsigned int>(__builtin_ia32_crc32di(sum, *src));
        offset += static_cast<unsigned int>(sizeof(bsls::Types::Uint64));
    }

    // Process any bytes remaining after the last aligned 8-byte block.
    while (offset < length) {
        sum = __builtin_ia32_crc32qi(sum, data[offset]);
        offset++;
    }
#  else
    // 32 bit implementation
    unsigned int sum    = crc;
    unsigned int offset = 0;

    // Process bytes one at a time until we reach a 4-byte boundary and can
    // start doing aligned 32-bit reads.
    unsigned int mask =   reinterpret_cast<unsigned int>(data)
                        & (sizeof(unsigned int) - 1);
    if (mask != 0) {
        unsigned int limit = bsl::min(length, sizeof(unsigned int) - mask);
        while (offset < limit) {
            sum = __builtin_ia32_crc32qi(sum, data[offset]);
            offset++;
        }
    }

    // Process 4 bytes at a time until we have fewer than 4 bytes left.
    while (offset + sizeof(unsigned int) <= length) {
        const unsigned int *src =  reinterpret_cast<const unsigned int*>(
                                                                data + offset);
        sum     = __builtin_ia32_crc32si(sum, *src);
        offset += sizeof(unsigned int);
    }

    // Process any bytes remaining after the last aligned 4-byte block.
    while (offset < length) {
        sum = __builtin_ia32_crc32qi(sum, data[offset]);
        offset++;
    }
#  endif // BSLS_PLATFORM_CPU_64_BIT

    return sum ^ ~0U; // XOROUT = true: Do a final XOR on output
}

#endif // BSLS_PLATFORM_CMP_GNU || BSLS_PLATFORM_CMP_CLANG

                        //-----------------------
                        // class Crc32cCalculator
                        //-----------------------

Crc32cCalculator::Crc32cFn Crc32cCalculator::g_crc32cFn = 0;

Crc32cCalculator::Crc32cCalculator()
{
#if defined(BSLS_PLATFORM_CPU_X86) || defined(BSLS_PLATFORM_CPU_X86_64)

#  if defined(BSLS_PLATFORM_CMP_CLANG)
#    define BDLDE_SSE4_2 bit_SSE42
#  elif defined(BSLS_PLATFORM_CMP_GNU)
#    define BDLDE_SSE4_2 bit_SSE4_2
#  endif

#  ifdef BDLDE_SSE4_2
    unsigned int eax, ebx, ecx, edx;
    __cpuid(1, eax, ebx, ecx, edx);

    if (ecx & BDLDE_SSE4_2) { // SSE 4.2 Support for CRC32-C

#    ifdef BSLS_PLATFORM_CPU_64_BIT
        BSLS_LOG_INFO("Using hardware version for CRC32-C computation "
                      "(SSE4.2 instructions available, 64-bit mode)");
        g_crc32cFn = crc32cSse64bit;

#    else
        BSLS_LOG_INFO("Using hardware version (serial) for CRC32-C "
                      "computation (SSE4.2 instructions available, "
                      "32-bit mode)");
        g_crc32cFn = crc32cHardwareSerial;
#    endif // BSLS_PLATFORM_CPU_64_BIT
#undef BDLDE_SSE4_2
    }
    else {
        BSLS_LOG_INFO("Using software version for CRC32-C computation "
                      "(SSE4.2 instructions not available)");
        g_crc32cFn = crc32cSoftware;
    }
#  else // Unsupported compiler
    BSLS_LOG_INFO("Using software version for CRC32-C computation "
                  "(unsupported compiler)");
    g_crc32cFn = crc32cSoftware;
#  endif
#elif defined(BSLS_PLATFORM_CPU_SPARC)
    if (is_sparc_crc32c_avail()) {
        BSLS_LOG_INFO("Using hardware version for CRC32-C computation "
                      "(sparc hardware support available)");
        g_crc32cFn = sparcHardware;
    } else {
        BSLS_LOG_INFO("Using software version for CRC32-C computation "
                      "(sparc hardware not available)");
        g_crc32cFn = crc32cSoftware;
    }
#else // Not supported architecture
    BSLS_LOG_INFO("Using software version for CRC32-C computation "
                  "(not an x86 architecture)");
    g_crc32cFn = crc32cSoftware;
#endif // BSLS_PLATFORM_CPU_X86 || BSLS_PLATFORM_CPU_X86_64
}

Crc32cCalculator& Crc32cCalculator::instance()
{
#if (__cplusplus >= 201103L)
    static Crc32cCalculator theInstance;
    return theInstance;
#else
    static Crc32cCalculator *theInstance_p = 0;
    BSLMT_ONCE_DO {
        static Crc32cCalculator theInstance;
        theInstance_p = &theInstance;
    }
    return *theInstance_p;
#endif
}

inline
unsigned int Crc32cCalculator::operator()(const unsigned char *data,
                                          unsigned int         length,
                                          unsigned int         crc) const
{
    return g_crc32cFn(data, length, crc);
}

}  // close unnamed namespace


                               // -------------
                               // struct Crc32c
                               // -------------

unsigned int Crc32c::calculate(const void   *data,
                               unsigned int  length,
                               unsigned int  crc)
{
    // PRECONDITIONS
    BSLS_ASSERT_SAFE(   (data || !length)
                     && "If 'data' is 0, then 'length' also must be 0");

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(length == 0)) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;
        return crc;                                                   // RETURN
    }

    Crc32cCalculator& calculator = Crc32cCalculator::instance();
    return calculator(static_cast<const unsigned char *>(data), length, crc);
}

                             // ------------------
                             // struct Crc32c_Impl
                             // ------------------

unsigned int
Crc32c_Impl::calculateSoftware(const void   *data,
                               unsigned int  length,
                               unsigned int  crc)
{
    // PRECONDITIONS
    BSLS_ASSERT_SAFE(   (data || !length)
                     && "If 'data' is 0, then 'length' also must be 0");

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(length == 0)) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;
        return crc;                                                   // RETURN
    }

    return crc32cSoftware(static_cast<const unsigned char *>(data),
                          length,
                          crc);
}

unsigned int
Crc32c_Impl::calculateHardwareSerial(const void   *data,
                                     unsigned int  length,
                                     unsigned int  crc)
{
    // PRECONDITIONS
    BSLS_ASSERT_SAFE(   (data || !length)
                     && "If 'data' is 0, then 'length' also must be 0");

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(length  == 0)) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;
        return crc;                                                   // RETURN
    }

    const unsigned char *dataUchar = static_cast<const unsigned char *>(data);
#if defined(BSLS_PLATFORM_CMP_GNU) || defined(BSLS_PLATFORM_CMP_CLANG)
    return crc32cHardwareSerial(dataUchar, length, crc);
#else
    return crc32cSoftware(dataUchar, length, crc);
#endif // BSLS_PLATFORM_CMP_GNU || BSLS_PLATFORM_CMP_CLANG
}

}  // close package namespace
}  // close enterprise namespace


// ----------------------------------------------------------------------------
// Copyright 2018 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
