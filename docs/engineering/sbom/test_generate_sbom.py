"""Unit tests for the SBOM generator's status and dependency parsing."""

from __future__ import annotations

import pathlib
import tempfile
import unittest

import generate_sbom


STATUS_TEXT = """Package: demo
Version: 1.0
Depends: dep
Architecture: arm64-osx-dynamic
Status: install ok installed

Package: demo
Feature: jit
Architecture: arm64-osx-dynamic
Status: install ok installed
"""


STATUS_WITH_FEATURE_DEPENDENCIES = """Package: demo
Version: 1.0
Depends: base
Architecture: arm64-osx-dynamic
Status: install ok installed

Package: demo
Feature: extra
Depends: extra-dep
Architecture: arm64-osx-dynamic
Status: install ok installed

Package: base
Version: 1.0
Architecture: arm64-osx-dynamic
Status: install ok installed

Package: extra-dep
Version: 1.0
Architecture: arm64-osx-dynamic
Status: install ok installed
"""


class StatusParsingTests(unittest.TestCase):
    def test_feature_records_are_not_packages(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            status = pathlib.Path(directory) / "status"
            status.write_text(STATUS_TEXT, encoding="utf-8")
            packages = generate_sbom.parse_status(status)
            features, feature_dependencies = generate_sbom.parse_feature_records(status)
        self.assertEqual(len(packages), 1)
        self.assertEqual(packages[0]["Package"], "demo")
        self.assertEqual(packages[0]["Version"], "1.0")
        self.assertEqual(features[("demo", "arm64-osx-dynamic")], ["jit"])
        self.assertEqual(feature_dependencies, {})

    def test_feature_dependencies_are_collected_for_the_base_package(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            status = pathlib.Path(directory) / "status"
            status.write_text(STATUS_WITH_FEATURE_DEPENDENCIES, encoding="utf-8")
            features, feature_dependencies = generate_sbom.parse_feature_records(
                status
            )
        key = ("demo", "arm64-osx-dynamic")
        self.assertEqual(features[key], ["extra"])
        self.assertEqual(feature_dependencies[key], ["extra-dep"])

    def test_package_lists_record_the_file_list_version(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            info = pathlib.Path(directory)
            (info / "demo_1.0_arm64-osx-dynamic.list").write_text(
                "arm64-osx-dynamic/lib/libdemo.a\n", encoding="utf-8"
            )
            lists, versions = generate_sbom.package_lists(info, "arm64-osx-dynamic")
        self.assertIn("arm64-osx-dynamic/lib/libdemo.a", lists["demo"])
        self.assertEqual(versions[("demo", "arm64-osx-dynamic")], "1.0")


class DependencyEdgeTests(unittest.TestCase):
    def test_edges_belong_to_the_declaring_package(self) -> None:
        packages = [
            {
                "Package": "app",
                "Version": "1",
                "Architecture": "arm64-osx-dynamic",
                "Depends": "mid",
            },
            {
                "Package": "mid",
                "Version": "1",
                "Architecture": "arm64-osx-dynamic",
                "Depends": "hostdep:arm64-osx",
            },
            {"Package": "hostdep", "Version": "1", "Architecture": "arm64-osx"},
        ]
        package_ids = {
            ("app", "arm64-osx-dynamic"): "SPDXRef-Package-app",
            ("mid", "arm64-osx-dynamic"): "SPDXRef-Package-mid",
            ("hostdep", "arm64-osx"): "SPDXRef-Package-hostdep",
        }
        relationships = generate_sbom.dependency_edges(
            packages, package_ids, "arm64-osx-dynamic"
        )
        self.assertEqual(
            {
                (relationship["spdxElementId"], relationship["relatedSpdxElement"])
                for relationship in relationships
            },
            {
                ("SPDXRef-Package-app", "SPDXRef-Package-mid"),
                ("SPDXRef-Package-mid", "SPDXRef-Package-hostdep"),
            },
        )
        self.assertEqual(len(relationships), 2)
        self.assertEqual(
            {relationship["spdxElementId"] for relationship in relationships},
            {"SPDXRef-Package-app", "SPDXRef-Package-mid"},
        )

    def test_feature_dependencies_are_merged_and_deduplicated(self) -> None:
        packages = [
            {
                "Package": "demo",
                "Version": "1",
                "Architecture": "arm64-osx-dynamic",
                "Depends": "base,extra-dep",
            },
            {"Package": "base", "Version": "1", "Architecture": "arm64-osx-dynamic"},
            {
                "Package": "extra-dep",
                "Version": "1",
                "Architecture": "arm64-osx-dynamic",
            },
        ]
        package_ids = {
            ("demo", "arm64-osx-dynamic"): "SPDXRef-Package-demo",
            ("base", "arm64-osx-dynamic"): "SPDXRef-Package-base",
            ("extra-dep", "arm64-osx-dynamic"): "SPDXRef-Package-extra-dep",
        }
        relationships = generate_sbom.dependency_edges(
            packages,
            package_ids,
            "arm64-osx-dynamic",
            {("demo", "arm64-osx-dynamic"): ["extra-dep"]},
        )
        self.assertEqual(
            {
                (relationship["spdxElementId"], relationship["relatedSpdxElement"])
                for relationship in relationships
            },
            {
                ("SPDXRef-Package-demo", "SPDXRef-Package-base"),
                ("SPDXRef-Package-demo", "SPDXRef-Package-extra-dep"),
            },
        )
        self.assertEqual(len(relationships), 2)

    def test_feature_qualified_dependency_keeps_triplet(self) -> None:
        packages = [
            {
                "Package": "app",
                "Version": "1",
                "Architecture": "arm64-osx-dynamic",
                "Depends": "dep[core]:arm64-osx",
            },
            {"Package": "dep", "Version": "1", "Architecture": "arm64-osx"},
        ]
        package_ids = {
            ("app", "arm64-osx-dynamic"): "SPDXRef-Package-app",
            ("dep", "arm64-osx"): "SPDXRef-Package-dep",
        }
        relationships = generate_sbom.dependency_edges(
            packages, package_ids, "arm64-osx-dynamic"
        )
        self.assertEqual(
            {
                (relationship["spdxElementId"], relationship["relatedSpdxElement"])
                for relationship in relationships
            },
            {("SPDXRef-Package-app", "SPDXRef-Package-dep")},
        )

    def test_explicit_triplet_mismatch_raises(self) -> None:
        packages = [
            {
                "Package": "app",
                "Version": "1",
                "Architecture": "arm64-osx-dynamic",
                "Depends": "dep:arm64-osx-dynamic",
            },
            {"Package": "dep", "Version": "1", "Architecture": "arm64-osx"},
        ]
        package_ids = {
            ("app", "arm64-osx-dynamic"): "SPDXRef-Package-app",
            ("dep", "arm64-osx"): "SPDXRef-Package-dep",
        }
        with self.assertRaises(SystemExit):
            generate_sbom.dependency_edges(
                packages, package_ids, "arm64-osx-dynamic"
            )

    def test_unresolved_dependency_raises(self) -> None:
        packages = [
            {
                "Package": "app",
                "Version": "1",
                "Architecture": "arm64-osx-dynamic",
                "Depends": "missingdep",
            }
        ]
        package_ids = {("app", "arm64-osx-dynamic"): "SPDXRef-Package-app"}
        with self.assertRaises(SystemExit):
            generate_sbom.dependency_edges(
                packages, package_ids, "arm64-osx-dynamic"
            )


class LicenseTests(unittest.TestCase):
    def test_feature_license_is_merged_into_the_declaration(self) -> None:
        metadata = {
            "license": "BSD-3-Clause WITH PCRE2-exception",
            "features": {"jit": {"license": "BSD-2-Clause"}},
        }
        value, _ = generate_sbom.license_declared(metadata, ["jit"], {})
        self.assertEqual(
            value, "BSD-3-Clause WITH PCRE2-exception AND BSD-2-Clause"
        )

    def test_or_expression_is_parenthesized_before_adding_a_feature_license(self) -> None:
        metadata = {
            "license": "LGPL-3.0-only OR GPL-3.0-only",
            "features": {"extra": {"license": "MIT OR Apache-2.0"}},
        }
        value, _ = generate_sbom.license_declared(metadata, ["extra"], {})
        self.assertEqual(
            value,
            "(LGPL-3.0-only OR GPL-3.0-only) AND (MIT OR Apache-2.0)",
        )


class PurlTests(unittest.TestCase):
    def test_purl_carries_triplet_and_port_version(self) -> None:
        package = {
            "Version": "10.49",
            "Port-Version": "2",
            "Architecture": "arm64-osx-dynamic",
        }
        self.assertEqual(
            generate_sbom.package_purl(package, "pcre2"),
            "pkg:generic/pcre2@10.49?triplet=arm64-osx-dynamic&port_version=2",
        )


if __name__ == "__main__":
    unittest.main()
