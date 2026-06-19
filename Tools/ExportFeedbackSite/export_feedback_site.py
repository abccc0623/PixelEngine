from __future__ import annotations

import json
import re
from datetime import datetime, timezone, timedelta
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
DOCS = ROOT / "Docs"
ITEMS = DOCS / "EngineFeedback" / "Items"
TASKS = DOCS / "EngineTasks" / "Todo"
SITE = ROOT / "site"


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8-sig", errors="replace")


def field(text: str, name: str, default: str = "") -> str:
    match = re.search(rf"^- {re.escape(name)}:\s*(.*)$", text, re.MULTILINE)
    return match.group(1).strip() if match else default


def section(text: str, name: str) -> str:
    match = re.search(
        rf"^## {re.escape(name)}\s*\n(.*?)(?=^## |\Z)",
        text,
        re.MULTILINE | re.DOTALL,
    )
    return match.group(1).strip() if match else ""


def discard_reason(text: str) -> str:
    match = re.search(r"^폐기 사유:[ \t]*([^\r\n]*)", text, re.MULTILINE)
    return match.group(1).strip() if match else ""


def parse_item(path: Path) -> dict[str, str]:
    text = read_text(path)
    return {
        "title": path.stem,
        "date": field(text, "날짜"),
        "area": field(text, "영역"),
        "status": field(text, "상태", "미검토"),
        "source": field(text, "출처"),
        "situation": section(text, "상황"),
        "problem": section(text, "문제"),
        "suggestion": section(text, "제안"),
        "discardReason": discard_reason(text),
        "path": path.relative_to(ROOT).as_posix(),
    }


def parse_task(path: Path) -> dict[str, str]:
    text = read_text(path)
    return {
        "title": re.sub(r"^#\s*", "", text.splitlines()[0]).strip() if text.splitlines() else path.stem,
        "area": field(text, "영역"),
        "status": field(text, "상태", "Todo"),
        "priority": field(text, "우선순위"),
        "source": field(text, "출처"),
        "created": field(text, "생성일"),
        "background": section(text, "배경"),
        "work": section(text, "작업"),
        "done": section(text, "완료 조건"),
        "path": path.relative_to(ROOT).as_posix(),
    }


def main() -> int:
    feedback = []
    if ITEMS.exists():
        feedback = [parse_item(path) for path in ITEMS.glob("*/*.md")]
    feedback.sort(key=lambda item: (item["date"], item["title"]), reverse=True)

    tasks = []
    if TASKS.exists():
        tasks = [parse_task(path) for path in TASKS.glob("*.md")]
    tasks.sort(key=lambda item: (item["created"], item["title"]), reverse=True)

    payload = {
        "generatedAt": datetime.now(timezone(timedelta(hours=9))).isoformat(timespec="seconds"),
        "feedback": feedback,
        "tasks": tasks,
    }

    SITE.mkdir(parents=True, exist_ok=True)
    (SITE / "feedback-data.json").write_text(
        json.dumps(payload, ensure_ascii=False, indent=2),
        encoding="utf-8",
        newline="\n",
    )
    print(f"Exported {len(feedback)} feedback items and {len(tasks)} tasks.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
