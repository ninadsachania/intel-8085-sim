#pragma once

struct Chip {
    //
    // Registers.
    //
    u8 reg_a = 0x00;
    u8 reg_b = 0x00;
    u8 reg_c = 0x00;
    u8 reg_d = 0x00;
    u8 reg_e = 0x00;
    u8 reg_h = 0x00;
    u8 reg_l = 0x00;

    u16 reg_sp  = 0x0000;
    u16 reg_pc  = 0x0000;
    u16 reg_psw = 0x0000;

    //
    // Flags.
    //
    bool flag_s  = false;
    bool flag_z  = false;
    bool flag_ac = false;
    bool flag_p  = false;
    bool flag_c  = false;

    //
    // Memory.
    //
    static constexpr s64 MEMORY_SIZE = 65536; // 64K.
    u8 memory[MEMORY_SIZE] = { 0 };

    //
    // IO Ports.
    //
    static constexpr s64 IO_PORTS_SIZE = 256;
    u8 io_ports[IO_PORTS_SIZE] = { 0 };
};
