#!/usr/bin/env python3

# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

import argparse
import json
import numpy as np
import csv
import yaml
import os
import seaborn as sns
import matplotlib.gridspec as gridspec
import matplotlib.pyplot as plt
import matplotlib as mpl
from collections import defaultdict
from pathlib import Path
from typing import List, Dict


def detect_format_from_extension(path: Path) -> str:
    ext = path.suffix.lower()
    if ext == ".csv":
        return "csv"
    elif ext == ".json":
        return "json"
    elif ext in [".yaml", ".yml"]:
        return "yaml"
    elif ext == ".txt":
        return "txt"
    else:
        return "txt"  # default


def parse_args():
    parser = argparse.ArgumentParser(
        description="Summarize Google Benchmark JSON output with percentiles.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )

    parser.add_argument(
        "-i",
        "--input",
        type=Path,
        required=True,
        help="Path to Google Benchmark JSON file.",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        default=None,
        help="Optional output file to write results.",
    )
    parser.add_argument(
        "--format",
        choices=["csv", "json", "yaml", "txt"],
        default=None,
        help="Output file format. If not set, inferred from output file extension.",
    )
    parser.add_argument(
        "--field",
        choices=["real_time", "cpu_time"],
        default="real_time",
        help="Which field to compute percentiles on.",
    )
    parser.add_argument(
        "--percentiles",
        type=float,
        nargs="+",
        default=[50, 75, 90, 95, 99, 99.9],
        help="List of percentiles to compute.",
    )
    parser.add_argument(
        "--plot", action="store_true", help="Generate bar chart of percentiles."
    )
    parser.add_argument(
        "--plot-file",
        type=Path,
        default=os.path.join(os.path.dirname(__file__), "percentiles.png"),
        help="Path to save the plot if --plot is set.",
    )

    return parser.parse_args()


def plot_benchmark_percentiles(
    rows: List[Dict],
    field: str,
    percentiles: List[float],
    plot_file: Path,
    font_scale: float = 1.0,
    plot_by_percentile: bool = True,
):
    mpl.rcParams.update({
        "font.family": "Noto Sans CJK JP",
        "font.size": 12,
        "axes.titlesize": 14,
        "axes.labelsize": 12,
        "xtick.labelsize": 10,
        "ytick.labelsize": 10,
        "legend.fontsize": 10,
    })
    sns.set_theme(style="whitegrid")
    sns.set_context("notebook", font_scale=font_scale)

    groups = defaultdict(list)
    for row in rows:
        prefix = row["Benchmark"].split("_")[0]
        groups[prefix].append(row)

    if plot_by_percentile:
        num_percentiles = len(percentiles)
        
        fig_width = max(12, len(rows) * 0.8)
        fig_height = max(4 * num_percentiles, 8)
        fig = plt.figure(figsize=(fig_width, fig_height))
        
        if num_percentiles > 1:
            n_subplots = num_percentiles * 2 - 1
            height_ratios = [1 if i % 2 == 0 else 0.1 for i in range(n_subplots)]
        else:
            n_subplots = 1
            height_ratios = [1]
        
        gs = gridspec.GridSpec(n_subplots, 1, height_ratios=height_ratios)
        
        all_values = []
        for row in rows:
            for p in percentiles:
                all_values.append(row[f"P{p}"])
        ymax = max(all_values) * 1.15 if all_values else 1
        
        group_colors = {}
        color_palette = sns.color_palette("Set2", len(groups))
        for i, group_name in enumerate(groups.keys()):
            group_colors[group_name] = color_palette[i]
        
        benchmark_names = [row["Benchmark"] for row in rows]
        short_names = []
        for name in benchmark_names:
            parts = name.split("_")
            if len(parts) > 2:
                short_names.append(f"{parts[0]}_{parts[-1]}")
            elif len(parts) == 2:
                short_names.append(parts[1])
            else:
                short_names.append(name)
        
        subplot_idx = 0
        for i, p in enumerate(percentiles):
            ax = fig.add_subplot(gs[subplot_idx])
            subplot_idx += 2 if num_percentiles > 1 else 1
            
            values = []
            colors = []
            for row in rows:
                values.append(row[f"P{p}"])
                group_prefix = row["Benchmark"].split("_")[0]
                colors.append(group_colors[group_prefix])
            
            bars = ax.bar(
                range(len(values)), 
                values, 
                color=colors,
                edgecolor="white",
                linewidth=1.0,
                alpha=0.8
            )
            
            for bar, value in zip(bars, values):
                height = bar.get_height()
                ax.text(
                    bar.get_x() + bar.get_width()/2, 
                    height + ymax * 0.005,
                    f'{value:.1f}',
                    ha='center', va='bottom',
                    fontsize=9, fontweight='bold'
                )
            
            ax.set_xticks(range(len(short_names)))
            ax.set_xticklabels(short_names, rotation=45, ha='right', fontsize=10)
            ax.set_ylim(0, ymax)
            ax.set_ylabel(f"{field} (ns)", fontsize=11, fontweight="bold")
            ax.set_title(f"P{p:.1f} Percentile", fontsize=14, fontweight="bold", pad=15)
            
            ax.grid(True, alpha=0.3, linestyle='--')
            ax.set_axisbelow(True)
        
        if len(groups) > 1:
            legend_elements = []
            for group_name, color in group_colors.items():
                legend_elements.append(
                    plt.Rectangle((0, 0), 1, 1, facecolor=color, alpha=0.8, label=group_name) # type: ignore
                )
            
            fig.legend(
                handles=legend_elements,
                loc='center right',
                bbox_to_anchor=(0.98, 0.5),
                title='Benchmark Groups',
                title_fontsize=12,
                fontsize=11,
                frameon=True,
                fancybox=True,
                shadow=True
            )
        
        fig.suptitle(
            "Benchmark Comparison by Percentile",
            fontsize=18,
            fontweight="bold",
            y=0.98
        )
        
        plt.tight_layout(rect=[0.0, 0.0, 0.85, 0.95]) # type: ignore
        
    else:
        num_groups = len(groups)
        n_subplots = num_groups * 2 - 1 if num_groups > 1 else 1
        
        fig = plt.figure(figsize=(14, max(3.5 * num_groups, 6)))
        height_ratios = [1 if i % 2 == 0 else 0.07 for i in range(n_subplots)]
        gs = gridspec.GridSpec(n_subplots, 1, height_ratios=height_ratios)
        
        subplot_idx = 0
        for i, (prefix, group_rows) in enumerate(groups.items()):
            ax = fig.add_subplot(gs[subplot_idx])
            subplot_idx += 2 if num_groups > 1 else 1
            
            bar_width = min(0.8 / len(group_rows), 0.08)
            x = np.arange(len(percentiles))
            colors = sns.color_palette("tab20", len(group_rows))
            
            group_values = []
            for row in group_rows:
                for p in percentiles:
                    group_values.append(row[f"P{p}"])
            ymax = max(group_values) * 1.1 if group_values else 1
            
            for j, (row, color) in enumerate(zip(group_rows, colors)):
                values = [row[f"P{p}"] for p in percentiles]
                ax.bar(
                    x + j * bar_width,
                    values,
                    bar_width,
                    label=row["Benchmark"],
                    color=color,
                    edgecolor="white",
                    alpha=0.8
                )
            
            ax.set_xticks(x + bar_width * (len(group_rows) - 1) / 2)
            ax.set_xticklabels([f"P{p:.1f}" for p in percentiles])
            ax.set_ylim(0, ymax)
            ax.set_ylabel(f"{field} (ns)", fontsize=11, fontweight="bold")
            ax.set_title(
                f"Benchmark Percentiles: {prefix}", 
                fontsize=16, 
                fontweight="bold"
            )
            ax.legend(
                loc="upper left", 
                bbox_to_anchor=(1.01, 1.0), 
                fontsize=11, 
                frameon=True
            )
            
            ax.grid(True, alpha=0.3, linestyle='--')
            ax.set_axisbelow(True)
        
        fig.suptitle(
            "Logging Benchmark Percentile Comparison",
            fontsize=18,
            fontweight="bold",
            y=0.98
        )
        
        plt.tight_layout(rect=[0.0, 0.0, 0.85, 0.95]) # type: ignore
    
    fig.savefig(plot_file, dpi=200, bbox_inches='tight', facecolor='white')
    plt.close(fig)
    print(f"📊 Plot saved to: {plot_file}")


def format_result(
    input: Path,
    output: Path | None,
    format: str | None,
    field: str,
    percentiles: list,
    plot: bool,
    plot_file: Path,
):
    if not os.path.isfile(input):
        print("input file not found")
        return 1
    with input.open() as f:
        data = json.load(f)

    results = defaultdict(list)

    for b in data["benchmarks"]:
        if b.get("run_type") == "iteration":
            name = b["name"]
            results[name].append(b[field])

    rows = []
    for name, times in results.items():
        times_np = np.array(times)
        row = {"Benchmark": name}
        for p in percentiles:
            row[f"P{p}"] = float(np.percentile(times_np, p))
        rows.append(row)

    # Console output
    for row in rows:
        print(f"\nBenchmark: {row['Benchmark']}")
        for p in percentiles:
            print(f"  P{p:2}: {row[f'P{p}']:.3f} ns")

    # Output to file
    if output:
        fmt = format or detect_format_from_extension(output)
        output.parent.mkdir(parents=True, exist_ok=True)

        if fmt == "csv":
            fieldnames = ["Benchmark"] + [f"P{p}" for p in percentiles]
            with output.open("w", newline="") as f:
                writer = csv.DictWriter(f, fieldnames=fieldnames)
                writer.writeheader()
                writer.writerows(rows)
        elif fmt == "json":
            with output.open("w") as f:
                json.dump(rows, f, indent=2)
        elif fmt == "yaml":
            with output.open("w") as f:
                yaml.safe_dump(rows, f, sort_keys=False)
        elif fmt == "txt":
            with output.open("w") as f:
                for row in rows:
                    f.write(f"Benchmark: {row['Benchmark']}\n")
                    for p in percentiles:
                        f.write(f"  P{p:2}: {row[f'P{p}']:.3f} ns\n")
                    f.write("\n")
        print(f"\n✅ Output written to: {output}")

    if plot:
        plot_benchmark_percentiles(
            rows,
            field,
            percentiles,
            plot_file,
            font_scale=1.2,
        )
    return 0


if __name__ == "__main__":
    import sys

    args = parse_args()
    sys.exit(
        format_result(
            args.input,
            args.output,
            args.format,
            args.field,
            args.percentiles,
            args.plot,
            args.plot_file,
        )
    )
