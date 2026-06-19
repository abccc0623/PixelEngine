from __future__ import annotations

import json
import os
import re
import subprocess
import sys
import urllib.error
import urllib.request
from datetime import datetime, timezone, timedelta
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
DOCS = ROOT / "Docs"
DAILY = DOCS / "EngineFeedback" / "Daily"
ITEMS = DOCS / "EngineFeedback" / "Items"

AREAS = {"엔진", "에디터", "클라이언트", "그래픽", "물리", "사운드"}


def read_text(path: Path, limit: int = 12000) -> str:
    if not path.exists():
        return ""
    text = path.read_text(encoding="utf-8", errors="replace")
    return text[:limit]


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8", newline="\n")


def run_git(args: list[str]) -> str:
    try:
        return subprocess.check_output(["git", *args], cwd=ROOT, text=True, encoding="utf-8", errors="replace")
    except Exception:
        return ""


def file_tree() -> str:
    files = run_git(["ls-files"])
    selected: list[str] = []
    prefixes = (
        "PixelSolution/PixelEngine/",
        "PixelSolution/PixelTool/",
        "PixelSolution/PixelGraphics/",
        "PixelSolution/Client/",
        "PixelSolution/Generate/",
        "PixelSolution/PixelMeta/",
    )
    for line in files.splitlines():
        if line.startswith(prefixes) and not any(skip in line for skip in ("/Import/", "/LSP/meta/", "/Library/")):
            selected.append(line)
    return "\n".join(selected[:260])


def recent_titles() -> str:
    if not ITEMS.exists():
        return ""
    titles = [path.stem for path in ITEMS.glob("*/*.md")]
    return "\n".join(sorted(titles)[-80:])


def code_context() -> str:
    paths = [
        "CODEX.md",
        "PixelSolution/PixelEngine/Scene.cpp",
        "PixelSolution/PixelEngine/Pool.cpp",
        "PixelSolution/PixelEngine/Pool.h",
        "PixelSolution/PixelEngine/PoolManager.cpp",
        "PixelSolution/PixelEngine/PoolManager.h",
        "PixelSolution/PixelEngine/Group.cpp",
        "PixelSolution/PixelEngine/GroupManager.cpp",
        "PixelSolution/PixelEngine/LuaCreate.cpp",
        "PixelSolution/PixelTool/ProjectPathService.cs",
        "PixelSolution/PixelGraphics/GraphicsCore.cpp",
    ]
    chunks = []
    for rel in paths:
        path = ROOT / rel
        text = read_text(path, 8000)
        if text:
            chunks.append(f"--- {rel} ---\n{text}")
    return "\n\n".join(chunks)


def today_kst() -> str:
    return datetime.now(timezone(timedelta(hours=9))).date().isoformat()


def openai_request(prompt: str) -> list[dict[str, str]]:
    api_key = os.environ.get("OPENAI_API_KEY", "").strip()
    if not api_key:
        raise RuntimeError("OPENAI_API_KEY GitHub Secret is required.")

    model = os.environ.get("OPENAI_MODEL", "gpt-4.1-mini")
    payload = {
        "model": model,
        "input": [
            {
                "role": "system",
                "content": (
                    "You generate concise Korean engine-development feedback for a side-project C++ game engine. "
                    "Return only valid JSON. Do not use markdown fences."
                ),
            },
            {"role": "user", "content": prompt},
        ],
        "text": {"format": {"type": "json_object"}},
    }
    data = json.dumps(payload).encode("utf-8")
    request = urllib.request.Request(
        "https://api.openai.com/v1/responses",
        data=data,
        headers={
            "Authorization": f"Bearer {api_key}",
            "Content-Type": "application/json",
        },
        method="POST",
    )
    try:
        with urllib.request.urlopen(request, timeout=90) as response:
            body = json.loads(response.read().decode("utf-8"))
    except urllib.error.HTTPError as exc:
        detail = exc.read().decode("utf-8", errors="replace")
        raise RuntimeError(f"OpenAI API request failed: {exc.code} {detail}") from exc

    text = body.get("output_text")
    if not text:
        parts = []
        for output in body.get("output", []):
            for content in output.get("content", []):
                if content.get("type") in {"output_text", "text"}:
                    parts.append(content.get("text", ""))
        text = "".join(parts)
    parsed = json.loads(text)
    items = parsed.get("feedback", parsed if isinstance(parsed, list) else [])
    if not isinstance(items, list):
        raise RuntimeError("OpenAI response did not contain a feedback array.")
    return [normalize_item(item) for item in items][:5]


def normalize_item(item: dict[str, str]) -> dict[str, str]:
    area = str(item.get("area", "엔진")).strip()
    if area not in AREAS:
        area = "엔진"
    return {
        "title": str(item.get("title", "")).strip()[:120],
        "area": area,
        "summary": str(item.get("summary", "")).strip(),
        "situation": str(item.get("situation", "")).strip(),
        "problem": str(item.get("problem", "")).strip(),
        "suggestion": str(item.get("suggestion", "")).strip(),
    }


def fallback_items() -> list[dict[str, str]]:
    return [
        {
            "title": "Pool 생성 경로와 Group 등록 정책을 분리해 확인하기 어려움",
            "area": "엔진",
            "summary": "Pool API가 실제 재사용 정책으로 이어지는지 확인하기 어려운 문제.",
            "situation": "Lua에서 반복 생성되는 투사체를 Pool로 만들고 재사용 흐름을 확인하려고 함.",
            "problem": "Pool 생성과 Group 등록의 책임이 섞이면 사용자가 어떤 API가 생명주기를 책임지는지 판단하기 어렵다.",
            "suggestion": "Pool 생성, Group 등록, 활성/비활성 전환의 책임을 함수 단위로 분리하고 로그로 확인 가능하게 만들면 좋음.",
        }
    ]


def build_prompt(day: str) -> str:
    return f"""
오늘 날짜는 {day}이다.
PixelEngine 프로젝트 기준으로 엔진 개발 피드백 5개를 생성해라.

규칙:
- 게임 플레이 감상이 아니라 엔진, 에디터, 클라이언트, 그래픽, 물리, 사운드 같은 개발/사용성 피드백이어야 한다.
- 실제 코드와 파일 구조를 근거로 삼아라.
- 확인되지 않은 기능을 이미 구현된 것처럼 단정하지 마라.
- 최근 생성 제목과 중복하지 마라.
- 각 항목은 상황, 문제, 제안 흐름을 가져야 한다.
- 영역은 엔진, 에디터, 클라이언트, 그래픽, 물리, 사운드 중 하나다.

반환 JSON 형식:
{{
  "feedback": [
    {{
      "title": "짧고 구체적인 제목",
      "area": "엔진",
      "summary": "대시보드에 보일 한 줄 요약",
      "situation": "어떤 작업을 하다가 마주칠 상황",
      "problem": "현재 방식의 불편하거나 위험한 점",
      "suggestion": "개선 제안"
    }}
  ]
}}

최근 생성 제목:
{recent_titles()}

프로젝트 파일 트리 일부:
{file_tree()}

코드/프로젝트 메모:
{code_context()}
""".strip()


def safe_filename(title: str) -> str:
    name = re.sub(r'[<>:"/\\|?*\r\n]+', " ", title).strip()
    name = re.sub(r"\s+", " ", name)
    return name[:120] or "feedback"


def item_link(day: str, title: str) -> str:
    return f"[[{title}]]"


def write_item(day: str, item: dict[str, str]) -> None:
    path = ITEMS / day / f"{safe_filename(item['title'])}.md"
    if path.exists():
        return
    content = f"""- 날짜: {day}
- 영역: {item['area']}
- 상태: 미검토
- 출처: [[{day}]]

## 결정
- [ ] 채택
- [ ] 폐기

폐기 사유:

## 상황
{item['situation']}

## 문제
{item['problem']}

## 제안
{item['suggestion']}
"""
    write_text(path, content)


def write_daily(day: str, items: list[dict[str, str]]) -> None:
    path = DAILY / f"{day}.md"
    blocks = []
    for item in items:
        blocks.append(
            f"""> [!note] {item['area']} · 미검토
> {item_link(day, item['title'])}
>
> {item['summary']}"""
        )
    content = f"""# {day} Engine Feedback

## 오늘의 피드백

{chr(10).join(blocks)}

## 상태 요약

| 상태 | 개수 |
| --- | ---: |
| 미검토 | {len(items)} |
| 채택 | 0 |
| 폐기 | 0 |
| 일감화 | 0 |
"""
    write_text(path, content)


def main() -> int:
    day = os.environ.get("FEEDBACK_DATE", "").strip() or today_kst()
    prompt = build_prompt(day)
    try:
        items = openai_request(prompt)
    except Exception:
        if os.environ.get("ALLOW_FALLBACK_FEEDBACK") == "1":
            items = fallback_items()
        else:
            raise
    if len(items) < 5:
        raise RuntimeError(f"Expected 5 feedback items, got {len(items)}.")
    for item in items:
        write_item(day, item)
    write_daily(day, items)
    print(f"Generated {len(items)} feedback items for {day}.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
