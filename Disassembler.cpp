#include "Disassembler.h"
#ifdef _WIN64
Disassembler::Disassembler() {
    memset(&decoder, 0, sizeof(decoder));
    memset(&formatter, 0, sizeof(formatter));

    ZydisDecoderInit(
        &decoder,
        ZYDIS_MACHINE_MODE_LEGACY_32,
        ZYDIS_ADDRESS_WIDTH_32);

    ZydisFormatterInit(&formatter,
        ZYDIS_FORMATTER_STYLE_INTEL);
}

#else
Disassembler::Disassembler() {
    memset(&decoder, 0, sizeof(decoder));
    memset(&formatter, 0, sizeof(formatter));

    ZydisDecoderInit(
        &decoder,
        ZYDIS_MACHINE_MODE_LEGACY_32,
        ZYDIS_ADDRESS_WIDTH_32);

    ZydisFormatterInit(&formatter,
        ZYDIS_FORMATTER_STYLE_INTEL);
}
#endif

std::vector<std::string> Disassembler::DecodeToString(uint8_t* instructionPointer, size_t length)
{
    uint8_t* ip = instructionPointer;
    std::vector<std::string> instructions;
    uintptr_t offset = 0;
    ZydisDecodedInstruction instruction;
    while (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, ip, length - offset, &instruction)))
    {
        char buffer[256];
        ZydisFormatterFormatInstruction(
            &formatter, &instruction, buffer, sizeof(buffer), (ZyanU64)ip);
        instructions.push_back(buffer);
        ip += instruction.length;
        offset += instruction.length;
    }
    return instructions;
}

std::vector<ZydisDecodedInstruction> Disassembler::Decode(uint8_t* instructionPointer, size_t length)
{
    uint8_t* ip = instructionPointer;
    std::vector<ZydisDecodedInstruction> instructions;
    uintptr_t offset = 0;
    ZydisDecodedInstruction instruction;
    while (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, ip, length - offset, &instruction)))
    {
        instructions.push_back(instruction);
        ip += instruction.length;
        offset += instruction.length;
    }
    return instructions;
}

size_t Disassembler::GetFunctionSize(uintptr_t functionAddress)
{
    const size_t maxLength = 4096;
    size_t funcSize = 0;
    uint8_t* ip = (uint8_t*)functionAddress;
    uintptr_t offset = 0;
    ZydisDecodedInstruction instruction;
    while (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, ip, maxLength - offset, &instruction)))
    {
        ip += instruction.length;
        offset += instruction.length;
        funcSize += instruction.length;
        if (instruction.mnemonic == ZYDIS_MNEMONIC_RET) break;
    }
    return funcSize;

}
