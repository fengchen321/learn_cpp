#!/usr/bin/env python3
import argparse
import csv
import json
import math
import statistics
import struct
import sys
from collections import defaultdict

HEADER_STRUCT = struct.Struct("<8sIIQQQII")
RECORD_STRUCT = struct.Struct("<QQIIIHH")


def load_sites(path):
    sites = {}
    with open(path, newline="", encoding="utf-8") as fh:
        reader = csv.DictReader(fh, delimiter="\t")
        for row in reader:
            sites[int(row["site_id"])] = row
    return sites


def load_records(path):
    with open(path, "rb") as fh:
        header_blob = fh.read(HEADER_STRUCT.size)
        if len(header_blob) != HEADER_STRUCT.size:
            raise ValueError("truncated perfbin header")

        magic, version, record_size, record_count, overwritten, dropped, shard_count, capacity = (
            HEADER_STRUCT.unpack(header_blob)
        )
        if magic.rstrip(b"\0") != b"PDTBIN1":
            raise ValueError("unexpected perfbin magic")
        if record_size != RECORD_STRUCT.size:
            raise ValueError("unexpected record size")

        records = []
        truncated = False
        for _ in range(record_count):
            raw = fh.read(RECORD_STRUCT.size)
            if len(raw) == 0:
                truncated = True
                break
            if len(raw) != RECORD_STRUCT.size:
                truncated = True
                break
            records.append(RECORD_STRUCT.unpack(raw))

    return {
        "version": version,
        "record_count": record_count,
        "parsed_record_count": len(records),
        "overwritten": overwritten,
        "dropped": dropped,
        "shard_count": shard_count,
        "capacity_per_shard": capacity,
        "truncated": truncated,
        "records": records,
    }


def trimmed_mean(values, ratio):
    if not values:
        return math.nan
    count = len(values)
    trim = int(count * ratio)
    if trim == 0 or trim * 2 >= count:
        return statistics.mean(values)
    ordered = sorted(values)
    return statistics.mean(ordered[trim : count - trim])


def summarize(records, sites):
    grouped = defaultdict(list)
    for start_ns, duration_ns, site_id, tid_hash, seq_no, cpu_hint, flags in records:
        grouped[site_id].append(duration_ns)

    summaries = []
    for site_id, durations in grouped.items():
        durations.sort()
        label = sites.get(site_id, {}).get("label", f"site_{site_id}")
        summaries.append(
            {
                "site_id": site_id,
                "label": label,
                "count": len(durations),
                "min_ns": durations[0],
                "p50_ns": durations[len(durations) // 2],
                "p90_ns": durations[min(len(durations) - 1, int(len(durations) * 0.90))],
                "p99_ns": durations[min(len(durations) - 1, int(len(durations) * 0.99))],
                "mean_ns": statistics.mean(durations),
                "trimmed_mean_ns": trimmed_mean(durations, 0.05),
                "max_ns": durations[-1],
            }
        )
    return sorted(summaries, key=lambda item: item["p50_ns"], reverse=True)


def main():
    parser = argparse.ArgumentParser(description="Analyze perf_duration_trace exports")
    parser.add_argument("sample_path", help="Path to .perfbin file")
    parser.add_argument("site_path", help="Path to .sites.tsv file")
    parser.add_argument("--json", action="store_true", help="Print JSON instead of table")
    args = parser.parse_args()

    try:
        sample_blob = load_records(args.sample_path)
        sites = load_sites(args.site_path)
    except (OSError, ValueError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2

    summaries = summarize(sample_blob["records"], sites)

    payload = {
        "record_count": sample_blob["record_count"],
        "parsed_record_count": sample_blob["parsed_record_count"],
        "overwritten": sample_blob["overwritten"],
        "dropped": sample_blob["dropped"],
        "shard_count": sample_blob["shard_count"],
        "capacity_per_shard": sample_blob["capacity_per_shard"],
        "truncated": sample_blob["truncated"],
        "sites": summaries,
    }

    if args.json:
        print(json.dumps(payload, indent=2))
        return 0

    if sample_blob["truncated"]:
        print("warning\tpartial record stream parsed", file=sys.stderr)

    print("label\tcount\tmin_ns\tp50_ns\tp90_ns\tp99_ns\tmean_ns\ttrimmed_mean_ns\tmax_ns")
    for row in summaries:
        print(
            f"{row['label']}\t{row['count']}\t{row['min_ns']}\t{row['p50_ns']}\t{row['p90_ns']}\t"
            f"{row['p99_ns']}\t{row['mean_ns']:.2f}\t{row['trimmed_mean_ns']:.2f}\t{row['max_ns']}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
