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

void reset_registers(Chip *chip) {
    chip->reg_a = 0x00;
    chip->reg_b = 0x00;
    chip->reg_c = 0x00;
    chip->reg_d = 0x00;
    chip->reg_e = 0x00;
    chip->reg_h = 0x00;
    chip->reg_l = 0x00;

    chip->reg_sp  = 0x0000;
    chip->reg_pc  = 0x0000;
    chip->reg_psw = 0x0000;
}

void reset_flags(Chip *chip) {
    chip->flag_s  = false;
    chip->flag_z  = false;
    chip->flag_ac = false;
    chip->flag_p  = false;
    chip->flag_c  = false;
}

void reset_io_ports(Chip *chip) {
    memset(chip->io_ports, 0, sizeof(chip->io_ports));
}

void reset_memory(Chip *chip) {
    memset(chip->memory, 0, sizeof(chip->memory));
}

void reset_all(Chip *chip) {
    reset_registers(chip);
    reset_flags(chip);
    reset_memory(chip);
    reset_io_ports(chip);
}
