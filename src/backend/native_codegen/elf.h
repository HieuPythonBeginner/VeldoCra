/**
 * @file elf.h
 * @brief ELF64 binary format implementation for native codegen
 */

#ifndef VELDOCRA_NATIVE_ELF_H
#define VELDOCRA_NATIVE_ELF_H

#include <vector>
#include <string>
#include <cstdint>

namespace veldanava {
namespace native_codegen {

// ELF64 data types
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;

// ELF header
struct Elf64_Ehdr {
    unsigned char e_ident[16];     // ELF identification
    Elf64_Half    e_type;          // Object file type
    Elf64_Half    e_machine;       // Machine type
    Elf64_Word    e_version;       // Object file version
    Elf64_Addr    e_entry;         // Entry point address
    Elf64_Off     e_phoff;         // Program header offset
    Elf64_Off     e_shoff;         // Section header offset
    Elf64_Word    e_flags;         // Processor-specific flags
    Elf64_Half    e_ehsize;        // ELF header size
    Elf64_Half    e_phentsize;     // Program header entry size
    Elf64_Half    e_phnum;         // Number of program header entries
    Elf64_Half    e_shentsize;     // Section header entry size
    Elf64_Half    e_shnum;         // Number of section header entries
    Elf64_Half    e_shstrndx;      // Section name string table index
};

// Program header
struct Elf64_Phdr {
    Elf64_Word  p_type;     // Type of segment
    Elf64_Word  p_flags;    // Segment attributes
    Elf64_Off   p_offset;   // Offset in file
    Elf64_Addr  p_vaddr;    // Virtual address in memory
    Elf64_Addr  p_paddr;    // Physical address (not used)
    Elf64_Xword p_filesz;   // Size in file
    Elf64_Xword p_memsz;    // Size in memory
    Elf64_Xword p_align;    // Alignment
};

// Section header
struct Elf64_Shdr {
    Elf64_Word  sh_name;      // Section name (index into string table)
    Elf64_Word  sh_type;      // Section type
    Elf64_Xword sh_flags;     // Section attributes
    Elf64_Addr  sh_addr;      // Virtual address in memory
    Elf64_Off   sh_offset;    // Offset in file
    Elf64_Xword sh_size;      // Section size
    Elf64_Word  sh_link;      // Link to other section
    Elf64_Word  sh_info;      // Miscellaneous information
    Elf64_Xword sh_addralign; // Address alignment
    Elf64_Xword sh_entsize;   // Size of entries, if section has table
};

// Symbol table entry
struct Elf64_Sym {
    Elf64_Word    st_name;  // Symbol name (index into string table)
    unsigned char st_info;  // Symbol type and binding
    unsigned char st_other; // Symbol visibility
    Elf64_Half    st_shndx; // Section index
    Elf64_Addr    st_value; // Symbol value
    Elf64_Xword   st_size;  // Symbol size
};

// ELF constants
const unsigned char ELFMAG[4] = {0x7f, 'E', 'L', 'F'};
const int EI_CLASS = 4;      // 64-bit
const int EI_DATA = 5;       // Little endian
const int EI_VERSION = 6;    // Version
const int EI_OSABI = 7;      // OS ABI
const int EI_ABIVERSION = 8; // ABI version

// Object file types
const Elf64_Half ET_EXEC = 2;     // Executable file
const Elf64_Half ET_DYN = 3;      // Shared object file

// Machine types
const Elf64_Half EM_X86_64 = 62;  // x86-64

// Program header types
const Elf64_Word PT_LOAD = 1;     // Loadable segment
const Elf64_Word PT_DYNAMIC = 2;  // Dynamic linking information

// Program header flags
const Elf64_Word PF_X = 1;        // Execute
const Elf64_Word PF_W = 2;        // Write
const Elf64_Word PF_R = 4;        // Read

// Section types
const Elf64_Word SHT_NULL = 0;          // Inactive section
const Elf64_Word SHT_PROGBITS = 1;      // Program data
const Elf64_Word SHT_SYMTAB = 2;        // Symbol table
const Elf64_Word SHT_STRTAB = 3;        // String table
const Elf64_Word SHT_RELA = 4;          // Relocation entries
const Elf64_Word SHT_HASH = 5;          // Symbol hash table
const Elf64_Word SHT_DYNAMIC = 6;       // Dynamic linking info
const Elf64_Word SHT_NOTE = 7;          // Notes
const Elf64_Word SHT_NOBITS = 8;        // No space in file
const Elf64_Word SHT_REL = 9;           // Relocation entries
const Elf64_Word SHT_SHLIB = 10;        // Reserved
const Elf64_Word SHT_DYNSYM = 11;       // Dynamic symbol table

// Section flags
const Elf64_Xword SHF_WRITE = 1;        // Writable
const Elf64_Xword SHF_ALLOC = 2;        // Occupies memory
const Elf64_Xword SHF_EXECINSTR = 4;    // Executable

// Symbol binding
const unsigned char STB_LOCAL = 0;      // Local symbol
const unsigned char STB_GLOBAL = 1;     // Global symbol
const unsigned char STB_WEAK = 2;       // Weak symbol

// Symbol types
const unsigned char STT_NOTYPE = 0;     // No type
const unsigned char STT_OBJECT = 1;     // Data object
const unsigned char STT_FUNC = 2;       // Function
const unsigned char STT_SECTION = 3;    // Section
const unsigned char STT_FILE = 4;       // Source file

// Special section indices
const Elf64_Half SHN_UNDEF = 0;         // Undefined section
const Elf64_Half SHN_ABS = 0xfff1;      // Absolute values
const Elf64_Half SHN_COMMON = 0xfff2;   // Common symbols

// ELF64 Generator class
class ELFGenerator {
public:
    ELFGenerator();

    // Add code section
    void add_code_section(const std::vector<uint8_t>& code, const std::string& name = ".text");

    // Add data section
    void add_data_section(const std::vector<uint8_t>& data, const std::string& name = ".data");

    // Add symbol
    void add_symbol(const std::string& name, Elf64_Addr value, Elf64_Xword size,
                   unsigned char binding, unsigned char type, Elf64_Half section);

    // Set entry point
    void set_entry_point(Elf64_Addr entry);

    // Generate final ELF binary
    std::vector<uint8_t> generate();

private:
    Elf64_Ehdr elf_header_;
    std::vector<Elf64_Phdr> program_headers_;
    std::vector<Elf64_Shdr> section_headers_;
    std::vector<std::vector<uint8_t>> section_data_;
    std::vector<std::string> section_names_;
    std::vector<Elf64_Sym> symbols_;
    std::vector<std::string> symbol_names_;
    std::string string_table_;
    std::string symbol_string_table_;

    Elf64_Half add_section_name(const std::string& name);
    Elf64_Word add_symbol_name(const std::string& name);
    void build_string_tables();
    void finalize_headers();
};

} // namespace native_codegen
} // namespace veldanava

#endif // VELDOCRA_NATIVE_ELF_H