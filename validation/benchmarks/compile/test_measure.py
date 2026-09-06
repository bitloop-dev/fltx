#!/usr/bin/env python3
"""Unit tests for the compile-probe manifest and source generator."""

from __future__ import annotations

import unittest

from expression_cases import EXPRESSION_CASES, EXPRESSION_STRESS_CASES, expression_for
from measure import INCLUDE_HEADERS, Probe, probes_for, source_for


class CompileProbeTests(unittest.TestCase):
    def test_expression_manifest_defines_the_complete_shape_matrix(self) -> None:
        self.assertEqual(len(EXPRESSION_CASES), 49)
        self.assertEqual(len(set(EXPRESSION_CASES.values())), 49)
        self.assertEqual(len(EXPRESSION_STRESS_CASES), 50)
        self.assertEqual(expression_for("mul_add"), "a * b + c")
        self.assertEqual(
            expression_for("all_supported_fused_kernels", 49),
            expression_for("all_supported_fused_kernels", 0),
        )

    def test_summary_and_expression_scopes_are_separate(self) -> None:
        summary = probes_for("summary", [25, 100], 64)
        matrix = probes_for("expressions", [25, 100], 64)
        self.assertEqual(sum(probe.category == "include" for probe in summary), len(INCLUDE_HEADERS) + 1)
        self.assertEqual(sum(probe.category == "expression-growth" for probe in summary), 4)
        self.assertEqual(len(matrix), len(EXPRESSION_STRESS_CASES) * 2)
        self.assertTrue(all(probe.category == "expression-shape" for probe in matrix))

    def test_generated_sources_distinguish_eager_and_expression_types(self) -> None:
        eager = source_for(Probe("expression-shape", "mul_add", "eager", 2))
        expression = source_for(Probe("expression-shape", "mul_add", "expression", 2))
        self.assertIn("bl::fqd_s eager_mul_add_0", eager)
        self.assertIn("bl::fqd expression_mul_add_0", expression)
        self.assertIn("#define FLTX_ENABLE_FQD_EXPRESSIONS 1", eager)
        self.assertIn("#define FLTX_ENABLE_FQD_EXPRESSIONS 1", expression)
        self.assertEqual(eager.count("return a * b + c;"), 2)
        self.assertEqual(expression.count("return a * b + c;"), 2)


if __name__ == "__main__":
    unittest.main()
