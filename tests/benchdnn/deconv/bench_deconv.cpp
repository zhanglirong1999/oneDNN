/*******************************************************************************
* Copyright 2018-2022 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sstream>

#include "dnnl_common.hpp"
#include "utils/parser.hpp"

#include "deconv/deconv.hpp"

namespace deconv {

void check_correctness(const settings_t &s) {
    for_(const auto &i_dir : s.dir)
    for_(const auto &i_cfg : s.cfg)
    for_(const auto &i_stag : s.stag)
    for_(const auto &i_wtag : s.wtag)
    for_(const auto &i_dtag : s.dtag)
    for_(const auto &i_alg : s.alg)
    for_(const auto &i_scales : s.scales)
    for_(const auto &i_zero_points : s.zero_points)
    for_(const auto &i_post_ops : s.post_ops)
    for_(const auto &i_scratchpad_mode : s.scratchpad_mode)
    for_(const auto &i_ctx_init : s.ctx_init)
    for_(const auto &i_ctx_exe : s.ctx_exe)
    for_(const auto &i_fpmath_mode : s.fpmath_mode)
    for (const auto &i_mb : s.mb) {
        auto attr = settings_t::get_attr(i_scales, i_zero_points, i_post_ops,
                i_scratchpad_mode, i_fpmath_mode);

        const prb_t prb(s.desc, i_dir, i_cfg, i_stag, i_wtag, i_dtag, i_alg,
                attr, i_ctx_init, i_ctx_exe, i_mb);
        std::stringstream ss;
        ss << prb;
        const std::string cpp_pstr = ss.str();
        const char *pstr = cpp_pstr.c_str();

        if (s.pattern && !match_regex(pstr, s.pattern)) return;
        BENCHDNN_PRINT(1, "run: %s\n", pstr);

        res_t res {};
        deconv::doit(&prb, &res);

        parse_result(res, pstr);

        if (is_bench_mode(PERF)) {
            perf_report_t pr(&prb, s.perf_template);
            pr.report(&res, pstr);
        }
    }
}

int bench(int argc, char **argv) {
    driver_name = "deconv";
    using namespace parser;
    static settings_t s;
    static const settings_t def {};
    for (; argc > 0; --argc, ++argv) {
        const bool parsed_options = parse_bench_settings(argv[0])
                || parse_batch(bench, argv[0])
                || parse_dir(s.dir, def.dir, argv[0])
                || parse_cfg(s.cfg, def.cfg, str2cfg, argv[0])
                || parse_tag(s.stag, def.stag, argv[0], "stag")
                || parse_tag(s.wtag, def.wtag, argv[0], "wtag")
                || parse_tag(s.dtag, def.dtag, argv[0], "dtag")
                || parse_alg(s.alg, def.alg, str2alg, argv[0])
                || parse_mb(s.mb, def.mb, argv[0])
                || parse_attr_scales(s.scales, argv[0])
                || parse_attr_zero_points(s.zero_points, argv[0])
                || parse_attr_post_ops(s.post_ops, argv[0])
                || parse_attr_scratchpad_mode(
                        s.scratchpad_mode, def.scratchpad_mode, argv[0])
                || parse_attr_fpmath_mode(
                        s.fpmath_mode, def.fpmath_mode, argv[0])
                || parse_ctx_init(s.ctx_init, def.ctx_init, argv[0])
                || parse_ctx_exe(s.ctx_exe, def.ctx_exe, argv[0])
                || parse_test_pattern_match(s.pattern, argv[0])
                || parse_perf_template(s.perf_template, s.perf_template_def,
                        s.perf_template_csv(), argv[0])
                || parse_reset(s, argv[0]) || parse_help(argv[0]);
        if (!parsed_options) {
            catch_unknown_options(argv[0]);

            SAFE(str2desc(&s.desc, argv[0]), CRIT);
            check_correctness(s);
        }
    }

    return parse_last_argument();
}

} // namespace deconv
