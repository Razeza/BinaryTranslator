typedef unsigned long        QWORD;
typedef unsigned int        DWORD;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;

char*  code      = nullptr;
size_t code_size = 0;

namespace ELF_HEADER
{
    constexpr DWORD signature              = 0x464C457F;   //Сигнатура elf файла
    constexpr BYTE  elf_class              = 0x2;          //elf64
    constexpr BYTE  endian_type            = 0x1;          //little endian
    constexpr WORD  format_version         = 0x1;          //

    constexpr BYTE  one_byte               = 8;            //8 zeros must be set

    constexpr WORD  file_type              = 0x2;          //file type
    constexpr WORD  machine_type           = 0x3E;         //machine type
    constexpr DWORD version                = 0x1;          //version

    constexpr DWORD entry                  = 0x400130;     //entry_point
    constexpr DWORD _0                     = 0x0;          //...

    constexpr DWORD program_header_offset  = 0x40;
//    constexpr DWORD section_header_offset  = 0x200;

    constexpr WORD elf_header_size         = 0x40;
    constexpr WORD size_of_program_header  = 0x38;

    constexpr WORD number_of_prog_entr     = 0x2;

    constexpr WORD section_header_size     = 0x40;
    constexpr WORD section_header_num_entr = 0x2;

    constexpr WORD index_of_shstrtab       = 0x5;
}

namespace PROGRAM_HEADER_CONST
{
    constexpr DWORD head_type         = 0x1;                //Loadable segment
    constexpr DWORD RE                = 0x5;                //Read and execute
    constexpr DWORD RW                = 0x6;                //Read and write
    constexpr QWORD virtual_address1  = 0x400000;
//    constexpr QWORD virtual_address2  = 0x600154;           //data needs initialisation
//    constexpr QWORD size_address1     = 0x00000000000000D2;
    constexpr QWORD size_address2     = 0x1;
    constexpr QWORD align             = 0x200000;
}

struct PROGRAM_HEADER
{
    DWORD flags;
    QWORD offset;
    QWORD virtual_address;
    QWORD memory_size;       //number of elements in memory

    PROGRAM_HEADER (const DWORD& flags,           const QWORD& offset,
                    const QWORD& virtual_address, const QWORD& memory_size):
            flags (flags),
            offset (offset),
            virtual_address (virtual_address),
            memory_size (memory_size)
    { set_program_header (); }

private:
    void set_program_header ();

};

namespace SECTION_HEADER_CONST
{
    constexpr DWORD head_type         = 0x1;
    constexpr QWORD alloc_and_execute = 0x6;
    constexpr QWORD alloc_and_write   = 0x3;

    constexpr QWORD text_offset       = 0x130;
    constexpr QWORD memory_offset     = 0x600000;


}

struct SECTION_HEADER
{
    QWORD flags;
    QWORD virtual_address;
    QWORD real_address;
    QWORD size;
    QWORD align;

    SECTION_HEADER (const QWORD& flags, const QWORD& virtual_address, const QWORD& real_adress,
                    const QWORD& size,  const QWORD& align):
            flags           (flags),
            virtual_address (virtual_address),
            real_address    (real_adress),
            size            (size),
            align           (align)
    { set_section_header (); }

private:
    void set_section_header ();
};

inline void set_zero_byte (const size_t& bytes)
{
    for (size_t i = 0; i < bytes; i++)
        code[code_size++] = 0x00;
}

inline void set_zero_byte_until (const size_t& until)
{
    for (; code_size < until;)
        code[code_size++] = 0x00;
}


template <class TypeValue>
inline void set_val (const TypeValue& value)
{
    *( (TypeValue*) &code[code_size]) = value;
    code_size += sizeof (TypeValue);
}

template<typename FirstValue, typename... Others>
inline void set_val (const FirstValue& first, const Others&... value)
{
    *( (FirstValue*) &code[code_size]) = first;
    code_size += sizeof (FirstValue);

    set_val (value...);
}




void PROGRAM_HEADER::set_program_header ()
{
    set_val (PROGRAM_HEADER_CONST::head_type, flags, offset, virtual_address, virtual_address, 
             memory_size, memory_size,        PROGRAM_HEADER_CONST::align);
}

void SECTION_HEADER::set_section_header ()
{
    set_val ( (DWORD) 0x0,  SECTION_HEADER_CONST::head_type, flags, virtual_address, 
              real_address, size,   (QWORD) 0x0, align,     (QWORD) 0x0);
}

void Make_elf_header (const DWORD& section_header_offset)
{
    using namespace ELF_HEADER;

    set_val (signature,               elf_class, endian_type, format_version,      (QWORD) 0x0, file_type,
             machine_type,            version,   entry,       (DWORD) 0x0,         program_header_offset,
             (DWORD) 0x0,             section_header_offset,  (QWORD) 0x0,         
             elf_header_size,         size_of_program_header, number_of_prog_entr, section_header_size,
             section_header_num_entr, index_of_shstrtab);
}

void Make_program_header (const QWORD& size_address1) //size_of_excutable_code
{
    using namespace PROGRAM_HEADER_CONST;

    PROGRAM_HEADER elf_program_headers[2] = {
            PROGRAM_HEADER (RE, 0x0,   virtual_address1,                    size_address1),
            PROGRAM_HEADER (RW, 0x0,   SECTION_HEADER_CONST::memory_offset, size_address2) };
}


void Make_section_header (const QWORD& adress1, const QWORD& size1, const QWORD& adress2, const QWORD& size2)
{
    using namespace SECTION_HEADER_CONST;

    SECTION_HEADER elf_section_headers[2] = {
            SECTION_HEADER (alloc_and_execute, 0x400000 + text_offset, text_offset,   size1, 0x10),
            SECTION_HEADER (alloc_and_write,   0x0                  , memory_offset, size2, 0x4) };

}

void Make_executable_code (const unsigned char* execut_code, size_t size)
{

    memcpy (&code[code_size], execut_code, size);
    code_size += size;
}