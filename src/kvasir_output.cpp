/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "output_format.h"
#include "pio_disassembler.h"

#include <algorithm>
#include <iostream>

struct kvasir_output : public output_format {
    struct factory {
        factory() { output_format::add(new kvasir_output()); }
    };

    kvasir_output() : output_format("kvasir") {}

    std::string get_description() override { return "C++ header suitable for use with Kvasir"; }

    void output_symbols(FILE* out, std::vector<compiled_source::symbol> const& symbols) {
        for(auto const& s : symbols) {
            if(!s.is_label) {
    fprintf(out, "static constexpr auto %s{%d};\n", s.name.c_str(), s.value);
            }
        }
        for(auto const& s : symbols) {
            if(s.is_label) {
                fprintf(out, "static constexpr auto offset_%s{%du};\n", s.name.c_str(), s.value);
            }
        }
    }

    int output(
      std::string              destination,
      std::vector<std::string> output_options,
      compiled_source const&   source) override {
        for(auto const& program : source.programs) {
            for(auto const& p : program.lang_opts) {
                if(p.first.size() >= name.size() && p.first.compare(0, name.size(), name) == 0) {
                    std::cerr << "warning: " << name << " does not support output options; "
                              << p.first << " lang_opt ignored.\n";
                }
            }
        }

        FILE* out = open_single_output(destination);
        if(!out) return 1;

        fprintf(out, "#pragma once\n");
        fprintf(out, "#include <array>\n");
        fprintf(out, "#include <cstdint>\n");
        fprintf(out, "\n");

        fprintf(out, "namespace Kvasir {\n");
        fprintf(out, "namespace Pio {\n");

        output_symbols(out, source.global_symbols);

        for(auto const& program : source.programs) {
            fprintf(out, "struct %sProgramm {\n", program.name.c_str());

            output_symbols(out, program.symbols);
            fprintf(out, "\n");

            fprintf(out, "static constexpr std::array Instructions{\n");
            for(int i = 0; i < (int)program.instructions.size(); i++) {
                auto const& inst = program.instructions[i];
                if(i == program.wrap_target) {
                    fprintf(out, "//     .wrap_target\n");
                }
                fprintf(
                  out,
                  "std::uint16_t{0x%04x}, // %2d: %s\n",
                  inst,
                  i,
                  disassemble(inst, program.sideset_bits_including_opt.get(), program.sideset_opt)
                    .c_str());
                if(i == program.wrap) {
                    fprintf(out, "//     .wrap\n");
                }
            }
            fprintf(out, "};\n");
            fprintf(out, "\n");

            fprintf(out, "static constexpr auto WrapTarget{%d};\n", program.wrap_target);
            fprintf(out, "static constexpr auto Wrap{%d};\n", program.wrap);

            fprintf(out, "\n");/*
            fprintf(out, "//static constexpr auto get_default_config(std::uint16_t offset) {\n");
            fprintf(out, "//pio_sm_config c = pio_get_default_sm_config();\n");
            fprintf(out, "//sm_config_set_wrap(&c, offset + WrapTarget, offset + Wrap);\n");

            if(program.sideset_bits_including_opt.is_specified()) {
                fprintf(
                  out,
                  "//sm_config_set_sideset(&c, %d, %s, %s);\n",
                  program.sideset_bits_including_opt.get(),
                  program.sideset_opt ? "true" : "false",
                  program.sideset_pindirs ? "true" : "false");
            }
            fprintf(out, "//return c;\n");
            fprintf(out, "//}\n");
*/
            fprintf(out, "\n");
            // todo maybe have some code blocks inside or outside here?
            for(auto const& o : program.code_blocks) {
                fprintf(out, "\n");
                if(o.first == name) {
                    for(auto const& contents : o.second) {
                        fprintf(out, "%s", contents.c_str());
                        fprintf(out, "\n");
                    }
                }
            }

            fprintf(out, "};\n");
        }
        fprintf(out, "}\n");
        fprintf(out, "}\n");
        if(out != stdout) {
            fclose(out);
        }
        return 0;
    }
};

static kvasir_output::factory creator;
