from __future__ import annotations

import argparse
import json
import re
import socketserver
from dataclasses import dataclass
from datetime import date
from http import HTTPStatus
from http.server import SimpleHTTPRequestHandler
from pathlib import Path
from urllib.parse import unquote, urlparse


ROOT = Path(__file__).resolve().parents[2]
DOCS = ROOT / "Docs"
ITEMS = DOCS / "EngineFeedback" / "Items"
TASKS = DOCS / "EngineTasks" / "Todo"
ADOPTED = DOCS / "EngineFeedback" / "Adopted"
DISCARDED = DOCS / "EngineFeedback" / "Discarded"


@dataclass
class Feedback:
    id: str
    title: str
    path: Path
    date: str
    area: str
    status: str
    source: str
    situation: str
    problem: str
    suggestion: str
    discard_reason: str
    accepted_checked: bool
    discarded_checked: bool


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8-sig")


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8", newline="\n")


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


def parse_feedback(path: Path) -> Feedback:
    text = read_text(path)
    rel = path.relative_to(ROOT).as_posix()
    return Feedback(
        id=rel,
        title=path.stem,
        path=path,
        date=field(text, "날짜"),
        area=field(text, "영역"),
        status=field(text, "상태", "미검토"),
        source=field(text, "출처"),
        situation=section(text, "상황"),
        problem=section(text, "문제"),
        suggestion=section(text, "제안"),
        discard_reason=discard_reason(text),
        accepted_checked=bool(re.search(r"^- \[[xX]\] 채택\s*$", text, re.MULTILINE)),
        discarded_checked=bool(re.search(r"^- \[[xX]\] 폐기\s*$", text, re.MULTILINE)),
    )


def all_feedback() -> list[Feedback]:
    if not ITEMS.exists():
        return []
    return sorted(
        (parse_feedback(path) for path in ITEMS.glob("*/*.md")),
        key=lambda item: (item.date, item.title),
        reverse=True,
    )


def to_json(item: Feedback) -> dict[str, object]:
    return {
        "id": item.id,
        "title": item.title,
        "date": item.date,
        "area": item.area,
        "status": item.status,
        "source": item.source,
        "situation": item.situation,
        "problem": item.problem,
        "suggestion": item.suggestion,
        "discardReason": item.discard_reason,
        "acceptedChecked": item.accepted_checked,
        "discardedChecked": item.discarded_checked,
    }


def slug(value: str) -> str:
    cleaned = re.sub(r"[^0-9A-Za-z가-힣]+", "-", value).strip("-")
    return cleaned[:80] or "task"


def update_status(text: str, status: str) -> str:
    if re.search(r"^- 상태:", text, re.MULTILINE):
        return re.sub(r"^- 상태:\s*.*$", f"- 상태: {status}", text, count=1, flags=re.MULTILINE)
    return f"- 상태: {status}\n{text}"


def update_checks(text: str, decision: str) -> str:
    accept = "[x]" if decision == "accept" else "[ ]"
    discard = "[x]" if decision == "discard" else "[ ]"
    text = re.sub(r"^- \[[ xX]\] 채택\s*$", f"- {accept} 채택", text, flags=re.MULTILINE)
    text = re.sub(r"^- \[[ xX]\] 폐기\s*$", f"- {discard} 폐기", text, flags=re.MULTILINE)
    return text


def update_discard_reason(text: str, reason: str) -> str:
    if re.search(r"^폐기 사유:", text, re.MULTILINE):
        return re.sub(r"^폐기 사유:[ \t]*[^\r\n]*", f"폐기 사유: {reason}", text, count=1, flags=re.MULTILINE)
    return text.replace("## 상황", f"폐기 사유: {reason}\n\n## 상황", 1)


def create_task(item: Feedback) -> Path:
    task_path = TASKS / f"{item.date}-{slug(item.title)}.md"
    if task_path.exists():
        return task_path
    content = f"""# {item.title}

- 영역: {item.area}
- 상태: Todo
- 우선순위: Medium
- 출처: [[{item.title}]]
- 생성일: {date.today().isoformat()}

## 배경
{item.situation}

{item.problem}

## 작업
{item.suggestion}

## 완료 조건
- 개선 방향이 코드나 문서로 반영된다.
- Lua/API/에디터 사용자가 변경된 동작을 확인할 수 있다.
"""
    write_text(task_path, content)
    return task_path


def append_unique(path: Path, title: str, block: str) -> None:
    existing = read_text(path) if path.exists() else f"# {path.stem}\n"
    if f"## {title}" in existing:
        return
    write_text(path, existing.rstrip() + "\n\n" + block.strip() + "\n")


def record_adopted(item: Feedback, task_path: Path) -> None:
    path = ADOPTED / f"{item.date}.md"
    block = f"""## {item.title}

- 영역: {item.area}
- 출처: [[{item.title}]]
- 생성 일감: [[{task_path.stem}]]

### 피드백 요약
{item.problem}
"""
    append_unique(path, item.title, block)


def record_discarded(item: Feedback, reason: str) -> None:
    path = DISCARDED / f"{item.date}.md"
    block = f"""## {item.title}

- 영역: {item.area}
- 폐기일: {date.today().isoformat()}
- 출처: [[{item.title}]]

### 피드백 요약
{item.problem}

### 폐기 사유
{reason or "폐기 사유 미작성"}
"""
    append_unique(path, item.title, block)


def decide(item_id: str, decision: str, reason: str = "") -> Feedback:
    path = (ROOT / item_id).resolve()
    if ROOT not in path.parents or path.suffix.lower() != ".md":
        raise ValueError("Invalid feedback id.")
    text = read_text(path)
    text = update_checks(text, decision)
    if decision == "accept":
        text = update_status(text, "일감화")
    elif decision == "discard":
        text = update_status(text, "폐기")
        text = update_discard_reason(text, reason)
    elif decision == "reset":
        text = update_status(text, "미검토")
        text = update_checks(text, "reset")
    else:
        raise ValueError("Invalid decision.")
    write_text(path, text)

    item = parse_feedback(path)
    if decision == "accept":
        record_adopted(item, create_task(item))
    elif decision == "discard":
        record_discarded(item, reason)
    return item


class Handler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(Path(__file__).parent / "web"), **kwargs)

    def send_json(self, payload: object, status: HTTPStatus = HTTPStatus.OK) -> None:
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self) -> None:
        parsed = urlparse(self.path)
        if parsed.path == "/api/feedback":
            self.send_json({"items": [to_json(item) for item in all_feedback()]})
            return
        super().do_GET()

    def do_POST(self) -> None:
        parsed = urlparse(self.path)
        match = re.fullmatch(r"/api/feedback/(.+)/decision", parsed.path)
        if not match:
            self.send_error(HTTPStatus.NOT_FOUND)
            return
        length = int(self.headers.get("Content-Length", "0"))
        data = json.loads(self.rfile.read(length).decode("utf-8") or "{}")
        try:
            item = decide(
                unquote(match.group(1)),
                str(data.get("decision", "")),
                str(data.get("reason", "")),
            )
        except Exception as exc:
            self.send_json({"error": str(exc)}, HTTPStatus.BAD_REQUEST)
            return
        self.send_json({"item": to_json(item)})


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", default=8765, type=int)
    args = parser.parse_args()
    with socketserver.ThreadingTCPServer((args.host, args.port), Handler) as server:
        print(f"Feedback dashboard: http://{args.host}:{args.port}")
        server.serve_forever()


if __name__ == "__main__":
    main()
