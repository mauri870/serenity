/*
 * Copyright (c) 2020, Andreas Kling <kling@serenityos.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Emulator.h"
#include "SoftCPU.h"
#include <AK/LexicalPath.h>
#include <AK/LogStream.h>
#include <AK/MappedFile.h>
#include <AK/StringBuilder.h>
#include <LibCore/ArgsParser.h>
#include <LibELF/Loader.h>
#include <getopt.h>
#include <pthread.h>
#include <string.h>

int main(int argc, char** argv, char** env)
{
    if (argc == 1) {
        out() << "usage: UserspaceEmulator <command>";
        return 0;
    }

    // FIXME: Allow specifying any command in $PATH instead of requiring a full executable path.
    const char* executable_path = argv[1];

    MappedFile mapped_file(executable_path);
    if (!mapped_file.is_valid()) {
        warn() << "Unable to map " << executable_path;
        return 1;
    }

    auto elf = ELF::Loader::create((const u8*)mapped_file.data(), mapped_file.size());

    Vector<String> arguments;
    for (int i = 1; i < argc; ++i) {
        arguments.append(argv[i]);
    }

    Vector<String> environment;
    for (int i = 0; env[i]; ++i) {
        environment.append(env[i]);
    }

    UserspaceEmulator::Emulator emulator(arguments, environment, move(elf));
    if (!emulator.load_elf())
        return 1;

    StringBuilder builder;
    builder.append("(UE) ");
    builder.append(LexicalPath(arguments[0]).basename());
    if (set_process_name(builder.string_view().characters_without_null_termination(), builder.string_view().length()) < 0) {
        perror("set_process_name");
        return 1;
    }
    int rc = pthread_setname_np(pthread_self(), builder.to_string().characters());
    if (rc != 0) {
        fprintf(stderr, "pthread_setname_np: %s\n", strerror(rc));
        return 1;
    }
    return emulator.exec();
}
