#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include "helpers/StringConversions.h"
#include "Zydis/Zydis.h"
#include "Zycore/Format.h"

class Disassembler
{
	ZydisDecoder decoder;
	ZydisFormatter formatter;
	ZydisFormatterBuffer formatbuffer;
public:
	Disassembler();
	std::vector<std::string> DecodeToString(uint8_t* instructionPointer, size_t length);
	std::vector<ZydisDecodedInstruction> Decode(uint8_t* instructionPointer, size_t length);
	size_t GetFunctionSize(uintptr_t functionAddress);
};

