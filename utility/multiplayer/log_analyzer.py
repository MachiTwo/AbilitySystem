#!/usr/bin/env python3
import re
import argparse
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(
        description="Multiplayer Log Analyzer for Ability System"
    )
    parser.add_argument(
        "--log-dir", type=str, required=True, help="Directory containing player logs"
    )
    parser.add_argument(
        "--generate-mermaid",
        action="store_true",
        help="Generate Mermaid sequence diagrams for sync errors",
    )
    parser.add_argument(
        "--gha-summary", action="store_true", help="Format output for GHA Step Summary"
    )
    args = parser.parse_args()

    log_path = Path(args.log_dir)
    if not log_path.exists():
        print(f"Error: Log directory {args.log_dir} does not exist.")
        return

    results = {}
    total_passed = 0
    total_failed = 0

    # regex for doctest output: "passed: 37 | failed: 0"
    doctest_re = re.compile(r"passed:\s+(\d+)\s+\|\s+failed:\s+(\d+)")

    for log_file in log_path.glob("*.log"):
        player_match = re.search(r"player_(\d+)", log_file.name)
        player_id = player_match.group(1) if player_match else "unknown"

        with open(log_file, "r") as f:
            content = f.read()

            matches = doctest_re.findall(content)
            if matches:
                p, f_val = map(int, matches[-1])  # Take the last summary line
                results[player_id] = {
                    "passed": p,
                    "failed": f_val,
                    "status": "✅ PASS" if f_val == 0 else "❌ FAIL",
                }
                total_passed += p
                total_failed += f_val
            else:
                results[player_id] = {
                    "passed": 0,
                    "failed": 0,
                    "status": "⚠️ NO DATA / CRASHED",
                }

    # Generate Markdown Report
    report = []
    report.append("# 🌐 Multiplayer Test Summary")
    report.append(f"\n**Total Results:** {total_passed} Passed, {total_failed} Failed")

    report.append("\n| Player ID | Status | Passed | Failed |")
    report.append("|-----------|--------|--------|--------|")

    for pid in sorted(results.keys(), key=lambda x: int(x) if x.isdigit() else 999):
        r = results[pid]
        report.append(f"| {pid} | {r['status']} | {r['passed']} | {r['failed']} |")

    if args.generate_mermaid and total_failed > 0:
        report.append("\n## 🔄 Sync Analysis")
        report.append("\n```mermaid")
        report.append("sequenceDiagram")
        report.append("    participant S as Server")
        report.append("    participant C as Clients")
        report.append("    S->>C: State Update (Sync Tag)")
        report.append("    Note over C: Prediction Check")
        report.append("    C-->>S: Rollback Required (Mismatch!)")
        report.append("```")

    report_content = "\n".join(report)
    print(report_content)

    # Save to file expected by GHA
    with open("utility/multiplayer/mp_coverage_report.md", "w") as f:
        f.write(report_content)


if __name__ == "__main__":
    main()
