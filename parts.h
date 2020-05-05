

char*  code      = nullptr;
size_t code_size = 0;

void pass_push    (const unsigned char* binary_code, size_t& i);
void pass_pop     (const unsigned char* binary_code, size_t& i);
void pass_mul     ();
void pass_sub     ();
void pass_in      ();
void pass_out     ();
void restore_abcd ();
void save_abcd    ();
void pass_jumps   (const unsigned char* binary_code, size_t& i, const int* label_binary_code);
void pass_call    (const unsigned char* binary_code, size_t& i, const int* label_binary_code);
void pass_ret     ();
void pass_end     ();

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

    constexpr DWORD virtual_memory_start   = 0x400000;     //entry_point
    constexpr DWORD entry                  = 0x148; 
    constexpr DWORD _0                     = 0x0;          //...

    constexpr DWORD program_header_offset  = 0x40;

    constexpr WORD elf_header_size         = 0x40;
    constexpr WORD size_of_program_header  = 0x38;

    constexpr WORD number_of_prog_entr     = 0x2;

    constexpr WORD section_header_size     = 0x40;
    constexpr WORD section_header_num_entr = 0x2;

    constexpr WORD index_of_shstrtab       = 0x5;

    const QWORD memory_place = 0x600130; 
    const QWORD memory_digit = 0x600138; 
}

namespace PROGRAM_HEADER_CONST
{
    constexpr DWORD head_type         = 0x1;                //Loadable segment
    constexpr DWORD RE                = 0x5;                //Read and execute
    constexpr DWORD RW                = 0x6;                //Read and write
    constexpr QWORD size_address2     = 0x18;
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

    constexpr QWORD text_offset       = 0x148;
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

    set_val (signature,               elf_class, endian_type, format_version,  (QWORD) 0x0, file_type,
             machine_type,            version,   virtual_memory_start + entry, (DWORD) 0x0, program_header_offset,
             (DWORD) 0x0,             section_header_offset,  (QWORD) 0x0,     elf_header_size, size_of_program_header, 
             number_of_prog_entr, section_header_size, section_header_num_entr, index_of_shstrtab);
}

void Make_program_header (const QWORD& size_address1) //size_of_excutable_code
{
    using namespace PROGRAM_HEADER_CONST;

    PROGRAM_HEADER elf_program_headers[2] = {
            PROGRAM_HEADER (RE, 0x0,   ELF_HEADER::virtual_memory_start,   size_address1),
            PROGRAM_HEADER (RW, 0x0,   SECTION_HEADER_CONST::memory_offset, size_address2) };
}


void Make_section_header (const QWORD& adress1, const QWORD& size1, const QWORD& adress2, const QWORD& size2)
{
    using namespace SECTION_HEADER_CONST;

    SECTION_HEADER elf_section_headers[2] = {
            SECTION_HEADER (alloc_and_execute, 0x400000 + text_offset, text_offset,   size1, 0x10),
            SECTION_HEADER (alloc_and_write,   0x0,                    memory_offset, size2, 0x4) };

}

void Make_executable_code (unsigned char* binary_code, size_t file_size, int* label_binary_code)
{

    size_t j = code_size;

    for (size_t i = 8; i < file_size; i++) // skip signature
    {
        label_binary_code[i] = code_size;
        
        switch (binary_code[i])
        {
            case PUSH:
            {
                pass_push (binary_code, i);
                break;
            }

            case POP:
            {
                pass_pop (binary_code, i);
                break;
            }

            case MUL:
            {
                pass_mul ();
                break;
            }

            case SUB:
            {
                pass_sub ();
                break;
            }

            case IN_:
            {
                pass_in ();
                break;
            }

            case OUT_:
            {
                pass_out ();
                break;
            }

            case JA:  { }
            case JAE: { }
            case JB:  { }
            case JBE: { }
            case JE:  { }
            case JNE: { }

            case JMP:
            {
                pass_jumps (binary_code, i, label_binary_code);
                break;
            }

            case CALL:
            {
                pass_call (binary_code, i, label_binary_code);
                break;
            }

            case RET:
            {
                pass_ret ();
                break;
            }

            case END:
            {
                pass_end ();
                break;
            }

            default:
                std::cout << "ERROR";


        }
    }

    pass_end ();

    FILE* deb = fopen ("debug", "wb");
    for (;j < code_size; j++)
        fprintf (deb, "%c", code[j]);
    fclose (deb); 

    free (binary_code);
    free (label_binary_code);
}

void pass_push (const unsigned char* binary_code, size_t& i)
{
    switch (binary_code[i + 1])
    {
        case INT_:
        {
            set_val (functions::push_byte);
            set_val ( (BYTE) *(int*)(&binary_code[i + 2]));
            i += sizeof (char) + sizeof (int);
            break;
        }
    
        case REG:
        {
            set_val ( (BYTE) (functions::push_reg + reg_compare[binary_code[i + 2]]));
            i += 2 * sizeof (char);
            break;
        }

        default:
        {
            std::cout << "ERROR";
        }

    }
}

void pass_pop (const unsigned char* binary_code, size_t& i)
{
    switch (binary_code[i + 1])
    {
    
        case REG:
        {
            set_val ( (BYTE) (functions::pop_reg + reg_compare[binary_code[i + 2]]));
            i += 2*sizeof (char);
            break;
        }

        default:
        {
            std::cout << "ERROR";
        }

    }
}



void pass_mul ()
{
    set_val (prefix::REX_WB, functions::mov_reg, rm_byte::rax_r14,     // mov r14, rax
             (BYTE) (functions::pop_reg + reg_compare[ax]),            // pop rax
             prefix::REX_B, (BYTE) (functions::pop_reg + r10),         // pop r10
             prefix::REX_WB, functions::mul, rm_byte::mul_r10,         // mul r10
             (BYTE) (functions::push_reg + reg_compare[ax]),           // push rax
             prefix::REX_WR, functions::mov_reg, rm_byte::r14_rax);    // mov rax, r14
}

void pass_sub ()
{
    set_val (prefix::REX_B,   (BYTE) (functions::pop_reg + r8),              // pop r8
             prefix::REX_B,   (BYTE) (functions::pop_reg + r9),              // pop r9
             prefix::REX_WRB, functions::sub, rm_byte::r9_r8,                // sub r9, r8
             prefix::REX_B,   (BYTE) (functions::push_reg + r9));            // push r9
} 

void save_abcd ()
{
    set_val ( (BYTE) (functions::push_reg + reg_compare[ax]),                           // push rax
              (BYTE) (functions::push_reg + reg_compare[bx]),                           // push rbx
              (BYTE) (functions::push_reg + reg_compare[cx]),                           // push rcx
              (BYTE) (functions::push_reg + reg_compare[dx]));                          // push rdx
}  

void restore_abcd ()
{
    set_val ( (BYTE) (functions::pop_reg + reg_compare[dx]),                            // pop rdx
              (BYTE) (functions::pop_reg + reg_compare[cx]),                            // pop rcx
              (BYTE) (functions::pop_reg + reg_compare[bx]),                            // pop rbx
              (BYTE) (functions::pop_reg + reg_compare[ax]));                           // pop rax
}

void pass_in ()
{
    save_abcd ();
    set_val ( (BYTE) (functions::mov_dig  + reg_compare[ax]), (DWORD) 0x3,              // mov eax, 3
              (BYTE) (functions::mov_dig  + reg_compare[bx]), (DWORD) 0x2,              // mov ebx, 2

              (BYTE) (functions::mov_dig  + reg_compare[cx]), 
                      (DWORD) ELF_HEADER::memory_place,                                 // mov ecx, virtual_memory

              (BYTE) (functions::mov_dig  + reg_compare[dx]), (DWORD) 0x1,              // mov rdx, 1
              functions::int80h);                                                       // int 80h

    restore_abcd ();

    set_val (prefix::word, functions::sub_mem, rm_byte::sib_follow_sm,                  //sub [virtual_memory], 30
             sib_byte::subq_my_sib, (DWORD) ELF_HEADER::memory_place, (BYTE) 0x30,

             functions::push_mem, rm_byte::sib_follow_p, 
             sib_byte::subq_my_sib,         //push [virtual_memory]  
             (DWORD) ELF_HEADER::memory_place);

    
}

void pass_out ()
{
    set_val (prefix::REX_B, (BYTE) (functions::pop_reg + r13));                         // pop r13
    save_abcd ();

    set_val ( (BYTE) (functions::mov_dig  + reg_compare[ax]), (DWORD) 0x4,              // mov eax, 4
              (BYTE) (functions::mov_dig  + reg_compare[bx]), (DWORD) 0x1,              // mov ebx, 1

              prefix::REX_WR, functions::mov_reg, rm_byte::sib_follow_sm,               // mov [virtual_memory], r13
              sib_byte::subq_my_sib, (DWORD) ELF_HEADER::memory_place,

              (BYTE) (functions::mov_dig  + reg_compare[cx]), 
              (DWORD) ELF_HEADER::memory_place,                                         // mov ecx, virtual_memory

              (BYTE) (functions::mov_dig  + reg_compare[dx]), (DWORD) 0x1,              // mov rdx, 1
              functions::int80h);
    restore_abcd ();

}

void pass_jumps (const unsigned char* binary_code, size_t& i, const int* label_binary_code)
{

    switch (binary_code[i])
    {
        case JMP:
        {
            set_val (jumps::jmp);
            break;
        }

        default:
        {
            set_val (prefix::REX_B,   (BYTE) (functions::pop_reg + r11),         // pop r11
                     prefix::REX_B,   (BYTE) (functions::pop_reg + r12),         // pop r12
                     prefix::REX_WRB, functions::cmp, rm_byte::r12_r11);         // cmp r12, r11
            
            switch (binary_code[i])
            {

                case JA:
                {
                    set_val (prefix::short_, jumps::ja);
                    break;
                }

                case JE:
                {
                    set_val (prefix::short_, jumps::je);
                    break;
                }

                default:
                {
                    std::cout << "ERROR";
                }

            }
                                                
        }
    }


    set_val ( (DWORD) (label_binary_code[*(int*)(&binary_code[i + 1]) + 8] - code_size - sizeof (int)));
    i += sizeof (int);
}

void pass_call (const unsigned char* binary_code, size_t& i, const int* label_binary_code)
{
    set_val (functions::call, (DWORD) (label_binary_code[*(int*)(&binary_code[i + 1]) + 8] - code_size - sizeof (int) - 1));

    i += 2*sizeof (int);
}

void pass_ret ()
{
    set_val (functions::ret);
}

void pass_end ()
{
    set_val ( (BYTE) (functions::mov_dig  + reg_compare[ax]), (DWORD) 0x1,              // mov eax, 3
              (BYTE) (functions::mov_dig  + reg_compare[bx]), (DWORD) 0x0,              // mov ebx, 2
              functions::int80h);
}









