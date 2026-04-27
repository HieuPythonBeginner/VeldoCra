/**
 * @file elf.cpp
 * @brief ELF64 binary format implementation
 */

#include "elf.h"
#include <cstring>
#include <iostream>

namespace veldanava {
namespace native_codegen {

ELFGenerator::ELFGenerator() {
    // Initialize ELF header
    std::memset(&elf_header_, 0, sizeof(Elf64_Ehdr));

    // ELF magic
    elf_header_.e_ident[0] = ELFMAG[0];
    elf_header_.e_ident[1] = ELFMAG[1];
    elf_header_.e_ident[2] = ELFMAG[2];
    elf_header_.e_ident[3] = ELFMAG[3];

    // 64-bit, little endian, version 1, System V ABI
    elf_header_.e_ident[EI_CLASS] = 2;      // ELFCLASS64
    elf_header_.e_ident[EI_DATA] = 1;       // ELFDATA2LSB
    elf_header_.e_ident[EI_VERSION] = 1;    // EV_CURRENT
    elf_header_.e_ident[EI_OSABI] = 0;      // ELFOSABI_SYSV
    elf_header_.e_ident[EI_ABIVERSION] = 0;

    // Executable file for x86-64
    elf_header_.e_type = ET_EXEC;
    elf_header_.e_machine = EM_X86_64;
    elf_header_.e_version = 1;

    // Header sizes
    elf_header_.e_ehsize = sizeof(Elf64_Ehdr);
    elf_header_.e_phentsize = sizeof(Elf64_Phdr);
    elf_header_.e_shentsize = sizeof(Elf64_Shdr);

    // Add null section (required)
    Elf64_Shdr null_section = {};
    section_headers_.push_back(null_section);
    section_names_.push_back("");
    section_data_.push_back({});
}

void ELFGenerator::add_code_section(const std::vector<uint8_t>& code, const std::string& name) {
    Elf64_Shdr section = {};
    section.sh_name = add_section_name(name);
    section.sh_type = SHT_PROGBITS;
    section.sh_flags = SHF_ALLOC | SHF_EXECINSTR;
    section.sh_size = code.size();
    section.sh_addralign = 16;  // 16-byte alignment for code

    section_headers_.push_back(section);
    section_names_.push_back(name);
    section_data_.push_back(code);
}

void ELFGenerator::add_data_section(const std::vector<uint8_t>& data, const std::string& name) {
    Elf64_Shdr section = {};
    section.sh_name = add_section_name(name);
    section.sh_type = SHT_PROGBITS;
    section.sh_flags = SHF_ALLOC | SHF_WRITE;
    section.sh_size = data.size();
    section.sh_addralign = 8;   // 8-byte alignment for data

    section_headers_.push_back(section);
    section_names_.push_back(name);
    section_data_.push_back(data);
}

void ELFGenerator::add_symbol(const std::string& name, Elf64_Addr value, Elf64_Xword size,
                             unsigned char binding, unsigned char type, Elf64_Half section) {
    Elf64_Sym symbol = {};
    symbol.st_name = add_symbol_name(name);
    symbol.st_info = (binding << 4) | (type & 0xf);
    symbol.st_other = 0;
    symbol.st_shndx = section;
    symbol.st_value = value;
    symbol.st_size = size;

    symbols_.push_back(symbol);
    symbol_names_.push_back(name);
}

void ELFGenerator::set_entry_point(Elf64_Addr entry) {
    elf_header_.e_entry = entry;
}

Elf64_Half ELFGenerator::add_section_name(const std::string& name) {
    // Section names will be added to string table later
    return static_cast<Elf64_Half>(string_table_.size());
}

Elf64_Word ELFGenerator::add_symbol_name(const std::string& name) {
    // Symbol names will be added to symbol string table later
    return static_cast<Elf64_Word>(symbol_string_table_.size());
}

void ELFGenerator::build_string_tables() {
    // Build section string table
    string_table_ = "\0";  // Null string at start
    for (const auto& name : section_names_) {
        string_table_ += name + "\0";
    }

    // Build symbol string table
    symbol_string_table_ = "\0";  // Null string at start
    for (const auto& name : symbol_names_) {
        symbol_string_table_ += name + "\0";
    }

    // Add string table sections
    {
        Elf64_Shdr shstrtab = {};
        shstrtab.sh_name = add_section_name(".shstrtab");
        shstrtab.sh_type = SHT_STRTAB;
        shstrtab.sh_size = string_table_.size();
        section_headers_.push_back(shstrtab);
        section_names_.push_back(".shstrtab");
        section_data_.push_back(std::vector<uint8_t>(string_table_.begin(), string_table_.end()));
    }

    {
        Elf64_Shdr strtab = {};
        strtab.sh_name = add_section_name(".strtab");
        strtab.sh_type = SHT_STRTAB;
        strtab.sh_size = symbol_string_table_.size();
        section_headers_.push_back(strtab);
        section_names_.push_back(".strtab");
        section_data_.push_back(std::vector<uint8_t>(symbol_string_table_.begin(), symbol_string_table_.end()));
    }

    // Add symbol table section
    if (!symbols_.empty()) {
        Elf64_Shdr symtab = {};
        symtab.sh_name = add_section_name(".symtab");
        symtab.sh_type = SHT_SYMTAB;
        symtab.sh_link = section_headers_.size() + 1;  // Link to .strtab
        symtab.sh_info = 1;  // One local symbol (null)
        symtab.sh_entsize = sizeof(Elf64_Sym);
        symtab.sh_size = symbols_.size() * sizeof(Elf64_Sym);

        section_headers_.push_back(symtab);
        section_names_.push_back(".symtab");

        // Convert symbols to bytes
        std::vector<uint8_t> sym_data;
        sym_data.reserve(symtab.sh_size);
        for (const auto& sym : symbols_) {
            uint8_t* bytes = reinterpret_cast<uint8_t*>(const_cast<Elf64_Sym*>(&sym));
            sym_data.insert(sym_data.end(), bytes, bytes + sizeof(Elf64_Sym));
        }
        section_data_.push_back(sym_data);
    }
}

void ELFGenerator::finalize_headers() {
    // Calculate offsets
    uint64_t offset = sizeof(Elf64_Ehdr);

    // Program headers (for now, one load segment)
    elf_header_.e_phoff = offset;
    elf_header_.e_phnum = 1;

    Elf64_Phdr phdr = {};
    phdr.p_type = PT_LOAD;
    phdr.p_flags = PF_R | PF_X;  // Read + Execute
    phdr.p_offset = 0;  // Will be set later
    phdr.p_vaddr = 0x400000;  // Standard load address
    phdr.p_paddr = 0x400000;
    phdr.p_align = 0x1000;  // Page alignment

    offset += sizeof(Elf64_Phdr);

    // Section data
    uint64_t virtual_addr = 0x400000;
    for (size_t i = 1; i < section_headers_.size(); ++i) {  // Skip null section
        auto& shdr = section_headers_[i];
        shdr.sh_offset = offset;

        if (shdr.sh_flags & SHF_ALLOC) {
            shdr.sh_addr = virtual_addr;
            virtual_addr += shdr.sh_size;
            if (virtual_addr % shdr.sh_addralign != 0) {
                virtual_addr += shdr.sh_addralign - (virtual_addr % shdr.sh_addralign);
            }
        }

        offset += section_data_[i].size();
    }

    // Update program header
    phdr.p_filesz = offset - elf_header_.e_phoff;
    phdr.p_memsz = virtual_addr - 0x400000;
    phdr.p_offset = elf_header_.e_phoff;
    program_headers_.push_back(phdr);

    // Section headers
    elf_header_.e_shoff = offset;
    elf_header_.e_shnum = section_headers_.size();
    elf_header_.e_shstrndx = section_headers_.size() - 2;  // .shstrtab index

    offset += section_headers_.size() * sizeof(Elf64_Shdr);

    // Update section name offsets now that string table is built
    for (size_t i = 1; i < section_names_.size(); ++i) {
        section_headers_[i].sh_name = 0;  // Will need proper calculation
    }
}

std::vector<uint8_t> ELFGenerator::generate() {
    build_string_tables();
    finalize_headers();

    std::vector<uint8_t> binary;

    // ELF header
    uint8_t* hdr_bytes = reinterpret_cast<uint8_t*>(&elf_header_);
    binary.insert(binary.end(), hdr_bytes, hdr_bytes + sizeof(Elf64_Ehdr));

    // Program headers
    for (const auto& phdr : program_headers_) {
        uint8_t* phdr_bytes = reinterpret_cast<uint8_t*>(const_cast<Elf64_Phdr*>(&phdr));
        binary.insert(binary.end(), phdr_bytes, phdr_bytes + sizeof(Elf64_Phdr));
    }

    // Section data
    for (const auto& data : section_data_) {
        binary.insert(binary.end(), data.begin(), data.end());
    }

    // Section headers
    for (const auto& shdr : section_headers_) {
        uint8_t* shdr_bytes = reinterpret_cast<uint8_t*>(const_cast<Elf64_Shdr*>(&shdr));
        binary.insert(binary.end(), shdr_bytes, shdr_bytes + sizeof(Elf64_Shdr));
    }

    return binary;
}

} // namespace native_codegen
} // namespace veldanava