import argparse
import pyopencl as cl
import numpy as np

# Charger et compiler le code OpenCL
KERNEL_CODE = """
__constant ushort g_awTea1LutA[8] = { 0xDA86, 0x85E9, 0x29B5, 0x2BC6, 0x8C6B, 0x974C, 0xC671, 0x93E2 };
__constant ushort g_awTea1LutB[8] = { 0x85D6, 0x791A, 0xE985, 0xC671, 0x2B9C, 0xEC92, 0xC62B, 0x9C47 };
__constant uchar g_abTea1Sbox[256] = {
    0x9B, 0xF8, 0x3B, 0x72, 0x75, 0x62, 0x88, 0x22, 0xFF, 0xA6, 0x10, 0x4D, 0xA9, 0x97, 0xC3, 0x7B,
    0x9F, 0x78, 0xF3, 0xB6, 0xA0, 0xCC, 0x17, 0xAB, 0x4A, 0x41, 0x8D, 0x89, 0x25, 0x87, 0xD3, 0xE3,
    0xCE, 0x47, 0x35, 0x2C, 0x6D, 0xFC, 0xE7, 0x6A, 0xB8, 0xB7, 0xFA, 0x8B, 0xCD, 0x74, 0xEE, 0x11,
    0x23, 0xDE, 0x39, 0x6C, 0x1E, 0x8E, 0xED, 0x30, 0x73, 0xBE, 0xBB, 0x91, 0xCA, 0x69, 0x60, 0x49,
    0x5F, 0xB9, 0xC0, 0x06, 0x34, 0x2A, 0x63, 0x4B, 0x90, 0x28, 0xAC, 0x50, 0xE4, 0x6F, 0x36, 0xB0,
    0xA4, 0xD2, 0xD4, 0x96, 0xD5, 0xC9, 0x66, 0x45, 0xC5, 0x55, 0xDD, 0xB2, 0xA1, 0xA8, 0xBF, 0x37,
    0x32, 0x2B, 0x3E, 0xB5, 0x5C, 0x54, 0x67, 0x92, 0x56, 0x4C, 0x20, 0x6B, 0x42, 0x9D, 0xA7, 0x58,
    0x0E, 0x52, 0x68, 0x95, 0x09, 0x7F, 0x59, 0x9C, 0x65, 0xB1, 0x64, 0x5E, 0x4F, 0xBA, 0x81, 0x1C,
    0xC2, 0x0C, 0x02, 0xB4, 0x31, 0x5B, 0xFD, 0x1D, 0x0A, 0xC8, 0x19, 0x8F, 0x83, 0x8A, 0xCF, 0x33,
    0x9E, 0x3A, 0x80, 0xF2, 0xF9, 0x76, 0x26, 0x44, 0xF1, 0xE2, 0xC4, 0xF5, 0xD6, 0x51, 0x46, 0x07,
    0x14, 0x61, 0xF4, 0xC1, 0x24, 0x7A, 0x94, 0x27, 0x00, 0xFB, 0x04, 0xDF, 0x1F, 0x93, 0x71, 0x53,
    0xEA, 0xD8, 0xBD, 0x3D, 0xD0, 0x79, 0xE6, 0x7E, 0x4E, 0x9A, 0xD7, 0x98, 0x1B, 0x05, 0xAE, 0x03,
    0xC7, 0xBC, 0x86, 0xDB, 0x84, 0xE8, 0xD1, 0xF7, 0x16, 0x21, 0x6E, 0xE5, 0xCB, 0xA3, 0x1A, 0xEC,
    0xA2, 0x7D, 0x18, 0x85, 0x48, 0xDA, 0xAA, 0xF0, 0x08, 0xC6, 0x40, 0xAD, 0x57, 0x0D, 0x29, 0x82,
    0x7C, 0xE9, 0x8C, 0xFE, 0xDC, 0x0F, 0x2D, 0x3C, 0x2E, 0xF6, 0x15, 0x2F, 0xAF, 0xE1, 0xEB, 0x3F,
    0x99, 0x43, 0x13, 0x0B, 0xE0, 0xA5, 0x12, 0x77, 0x5D, 0xB3, 0x38, 0xD9, 0xEF, 0x5A, 0x01, 0x70
};

inline ulong tea1_expand_iv(uint dwShortIv) {
    uint dwXorred = dwShortIv ^ 0x96724FA1;
    dwXorred = (dwXorred << 8) | (dwXorred >> 24);
    ulong qwIv = ((ulong)dwShortIv << 32) | dwXorred;
    printf("Debug: Expanded IV = 0x%016lX\\n", qwIv);
    printf("Debug: dwXorred = 0x%08X\\n", dwXorred);
    printf("Debug: qwIv = 0x%016lX\\n", qwIv);
    return (qwIv >> 8) | (qwIv << 56);
}

inline uchar tea1_state_word_to_newbyte(ushort wSt, __constant ushort* awLut) {
    uchar bSt0 = wSt & 0xFF;
    uchar bSt1 = (wSt >> 8) & 0xFF;
    uchar bDist;
    uchar bOut = 0;

    for (int i = 0; i < 8; i++) {
        bDist = ((bSt0 >> 7) & 1) | ((bSt0 << 1) & 2) | ((bSt1 << 1) & 12);
        if (awLut[i] & (1 << bDist)) {
            bOut |= (1 << i);
        }
        bSt0 = ((bSt0 >> 1) | (bSt0 << 7));
        bSt1 = ((bSt1 >> 1) | (bSt1 << 7));
    }

    return bOut;
}

inline uchar tea1_reorder_state_byte(uchar bStByte) {
    uchar bOut = 0;
    bOut |= ((bStByte << 6) & 0x40);
    bOut |= ((bStByte << 1) & 0x20);
    bOut |= ((bStByte << 2) & 0x08);
    bOut |= ((bStByte >> 3) & 0x14);
    bOut |= ((bStByte >> 2) & 0x01);
    bOut |= ((bStByte >> 5) & 0x02);
    bOut |= ((bStByte << 4) & 0x80);
    return bOut;
}

int tea1_init_key_register(const uchar *lpKey) {
    int dwResult = 0;
    for (int i = 0; i < 10; i++) {
        dwResult = (dwResult << 8) | g_abTea1Sbox[((dwResult >> 24) ^ lpKey[i] ^ dwResult) & 0xff];
    }
    return dwResult;
}

void tea1_inner(ulong qwIvReg, uint dwKeyReg, uint dwNumKsBytes, __global uchar *lpKsOut) {
    uint dwNumSkipRounds = 54;

    for (int i = 0; i < dwNumKsBytes; i++) {
        for (int j = 0; j < dwNumSkipRounds; j++) {
            // Step 1: Derive a non-linear feedback byte through sbox and feed back into key register
            uchar bSboxOut = g_abTea1Sbox[((dwKeyReg >> 24) ^ dwKeyReg) & 0xff];
            dwKeyReg = (dwKeyReg << 8) | bSboxOut;

            // Step 2: Compute 3 bytes derived from current state
            uchar bDerivByte12 = tea1_state_word_to_newbyte((qwIvReg >>  8) & 0xffff, g_awTea1LutA);
            uchar bDerivByte56 = tea1_state_word_to_newbyte((qwIvReg >> 40) & 0xffff, g_awTea1LutB);
            uchar bReordByte4  = tea1_reorder_state_byte((qwIvReg >> 32) & 0xff);

            // Step 3: Combine current state with state derived values, and xor in key derived sbox output
            uchar bNewByte = (bDerivByte56 ^ (qwIvReg >> 56) ^ bReordByte4 ^ bSboxOut) & 0xff;
            uchar bMixByte = bDerivByte12;

            // Step 4: Update lfsr: leftshift 8, feed/mix in previously generated bytes
            qwIvReg = ((qwIvReg << 8) ^ ((ulong)bMixByte << 32)) | bNewByte;
        }

        lpKsOut[i] = (qwIvReg >> 56);
        dwNumSkipRounds = 19;
    }
}

void tea1(uint dwFrameNumbers, const uchar *lpKey, uint dwNumKsBytes, __global uchar *lpKsOut) {
    // Initialize IV and key register
    ulong qwIvReg = tea1_expand_iv(dwFrameNumbers);
    uint dwKeyReg = tea1_init_key_register(lpKey);

    // Invoke actual TEA1 core function
    tea1_inner(qwIvReg, dwKeyReg, dwNumKsBytes, lpKsOut);
}


typedef struct {
    uchar  tn;  // timeslot, 1 to 4
    uchar  fn;  // frame, 1 to 18
    uchar  mn;  // multiframe, 1 to 60
    uchar  dir; // 0 or 1; 0 = downlink, 1 = uplink
    ushort hn;  // hyperframe, 0 to 0xFFFF, although only 15 bits are used
    uint eck[4];  // hyperframe, 0 to 0xFFFF, although only 15 bits are used
} FrameNumbers;

uint build_iv(FrameNumbers *f) {
    return ((f->tn - 1) | (f->fn << 2) | (f->mn << 7) | ((f->hn & 0x7FFF) << 13) | (f->dir << 28));
}





__kernel void gen_ks(__global FrameNumbers* frame_data,
                     __global uchar* output) {
    int ks_len = 54;
    int gid = get_global_id(0);

    // Load frame parameters for this work item
    __global FrameNumbers* f = &frame_data[gid];

    // Build IV
    uint dwIv = build_iv(f);

    // Expand IV
    ulong qwIvReg = tea1_expand_iv(dwIv);

    // Initialize counter for eck
    uint counter = 0x00000000;

    // Increment eck values in a while loop
    while (counter <= 0x000FFFFF) {
        // Convert counter to 4-byte eck array
        uint eck[4];
        eck[0] = (counter >> 24) & 0xFF;
        eck[1] = (counter >> 16) & 0xFF;
        eck[2] = (counter >> 8) & 0xFF;
        eck[3] = counter & 0xFF;

        // Initialize key register using eck
        uint dwKeyReg = (eck[0] << 24) |
                        (eck[1] << 16) |
                        (eck[2] << 8) |
                        eck[3];

        // Generate keystream
        __global uchar* ks_out = output + gid * ks_len;
        tea1_inner(qwIvReg, dwKeyReg, ks_len, ks_out);

        // Increment the counter
        counter++;
        printf("%X\\n",counter);
    }
}








"""

def prepare_key_stream(tea_type, hn, mn, fn, sn, direction, eck, key_length):
    """
    Prepare and execute the OpenCL kernel to generate the keystream.
    """
    # Initialize OpenCL context and command queue
    context = cl.create_some_context()
    queue = cl.CommandQueue(context)

    # Build the OpenCL program
    program = cl.Program(context, KERNEL_CODE).build()

    # Define the FrameNumbers structure as a numpy dtype (with proper alignment)
    frame_numbers_dtype = np.dtype([
        ("tn", np.uint8),       # 1 byte
        ("fn", np.uint8),       # 1 byte
        ("mn", np.uint8),       # 1 byte
        ("dir", np.uint8),      # 1 byte (moved next to align with hn)
        ("hn", np.uint16),      # 2 bytes
        ("eck", np.uint32),     # Encryption key (4 bytes)
        ("padding", np.uint16)  # Padding to align the struct to 16 bytes
    ])

    # Check the size of the structure
    print("FrameNumbers size (bytes):", frame_numbers_dtype.itemsize)

    # Prepare the FrameNumbers data
    frame_data = np.array([(sn, fn, mn, direction, hn, eck, 0)], dtype=frame_numbers_dtype)
    frame_buf = cl.Buffer(context, cl.mem_flags.READ_ONLY | cl.mem_flags.COPY_HOST_PTR, hostbuf=frame_data)

    # Prepare other input data
    num_bytes_buf = cl.Buffer(context, cl.mem_flags.READ_ONLY | cl.mem_flags.COPY_HOST_PTR, hostbuf=np.array([key_length], dtype=np.uint32))
    output_buf = cl.Buffer(context, cl.mem_flags.WRITE_ONLY, size=key_length)

    # Debug buffer
    debug_buffer = cl.Buffer(context, cl.mem_flags.WRITE_ONLY, size=1024)

    # Set the kernel arguments
    kernel = program.gen_ks
    kernel.set_arg(0, frame_buf)       # FrameNumbers buffer
    kernel.set_arg(1, output_buf)     # Output buffer

    # Execute the kernel
    cl.enqueue_nd_range_kernel(queue, kernel, (1,), None)

    # Read back the output
    key_stream = np.empty(key_length, dtype=np.uint8)
    cl.enqueue_copy(queue, key_stream, output_buf).wait()

    # Optional: Read the debug buffer
    debug_data = np.empty(1024, dtype=np.uint8)
    cl.enqueue_copy(queue, debug_data, debug_buffer).wait()
    print("Debug Data:", debug_data)

    return key_stream

def main():
    """
    Main entry point for the script.
    """
    # Argument parser setup
    parser = argparse.ArgumentParser(description="Generate TEA keystream.")
    parser.add_argument("tea_type", type=int, help="TEA type (1, 2, or 3)")
    parser.add_argument("hn", type=int, help="Hyperframe number (hn)")
    parser.add_argument("mn", type=int, help="Multiframe number (mn)")
    parser.add_argument("fn", type=int, help="Frame number (fn)")
    parser.add_argument("sn", type=int, help="Slot number (sn)")
    parser.add_argument("direction", type=int, help="Direction (0=downlink, 1=uplink)")
    parser.add_argument("eck", type=str, help="Encryption key (8 hex digits)")
    parser.add_argument("--key_length", type=int, default=64, help="Length of the keystream to generate (default: 64 bytes)")

    args = parser.parse_args()

    # Validate the encryption key (eck)
    try:
        eck = int(args.eck, 16)
    except ValueError:
        print("Error: `eck` must be a valid 8-character hex string.")
        return

    # Inform the user about the generation
    print(f"Generating keystream for TEA type {args.tea_type} with frame: hn={args.hn}, mn={args.mn}, fn={args.fn}, sn={args.sn}, dir={args.direction}, eck={args.eck}")

    # Generate the keystream
    ks_out = prepare_key_stream(args.tea_type, args.hn, args.mn, args.fn, args.sn, args.direction, eck, args.key_length)

    # Output the result
    print("Generated keystream:", ks_out.tobytes().hex())

if __name__ == "__main__":
    main()
