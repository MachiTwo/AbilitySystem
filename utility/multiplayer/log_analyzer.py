import os
import glob
import re
from datetime import datetime


def analyze_mp_logs():
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    log_files = glob.glob(os.path.join(project_root, "doctest_mp_player_*.log"))

    if not log_files:
        print("[MP LOG ANALYZER] No multiplayer log files found.")
        return

    print(f"[MP LOG ANALYZER] Found {len(log_files)} log files. Generating report...")

    report_path = os.path.join(
        project_root, "utility", "multiplayer", "mp_coverage_report.md"
    )

    total_assertions = 0
    total_passed = 0
    total_failed = 0
    player_stats = {}

    for log_path in log_files:
        player_name = os.path.basename(log_path).replace(".log", "")
        player_stats[player_name] = {
            "assertions": 0,
            "passed": 0,
            "failed": 0,
            "errors": [],
        }

        with open(log_path, "r", encoding="utf-8", errors="replace") as f:
            content = f.read()

            # Extract Doctest summary
            # Example: [doctest] assertions: 130 | 130 passed | 0 failed |
            match = re.search(
                r"\[doctest\] assertions:\s*(\d+)\s*\|\s*(\d+)\s*passed\s*\|\s*(\d+)\s*failed",
                content,
            )
            if match:
                assertions = int(match.group(1))
                passed = int(match.group(2))
                failed = int(match.group(3))

                player_stats[player_name]["assertions"] = assertions
                player_stats[player_name]["passed"] = passed
                player_stats[player_name]["failed"] = failed

                total_assertions += assertions
                total_passed += passed
                total_failed += failed

            # Extract errors
            errors = re.findall(r"(ERROR:.*|FAILED:.*)", content)
            player_stats[player_name]["errors"] = errors[:5]  # Keep top 5 errors

    # Write Markdown Report
    with open(report_path, "w", encoding="utf-8") as f:
        f.write("# 🌐 Multiplayer Test Coverage Report\n\n")
        f.write(f"**Generated:** {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n")

        f.write("## 📊 Global Summary\n")
        f.write(f"- **Total Players Simulated:** {len(log_files)}\n")
        f.write(f"- **Total Assertions:** {total_assertions}\n")
        f.write(f"- **Passed:** {total_passed} ✅\n")
        f.write(f"- **Failed:** {total_failed} ❌\n\n")

        f.write("## 👤 Player Breakdown\n\n")
        f.write("| Player | Assertions | Passed | Failed | Status |\n")
        f.write("|---|---|---|---|---|\n")

        for p, stats in sorted(player_stats.items()):
            status = (
                "🟢 PASS"
                if stats["failed"] == 0 and stats["assertions"] > 0
                else "🔴 FAIL"
            )
            f.write(
                f"| `{p}` | {stats['assertions']} | {stats['passed']} | {stats['failed']} | {status} |\n"
            )

        f.write("\n## ⚠️ Error Traces (Top 5 per Player)\n\n")
        for p, stats in sorted(player_stats.items()):
            if stats["errors"]:
                f.write(f"### {p}\n```\n")
                for err in stats["errors"]:
                    f.write(f"{err}\n")
                f.write("```\n\n")

    print(f"[MP LOG ANALYZER] Report generated at: {report_path}")


if __name__ == "__main__":
    analyze_mp_logs()
