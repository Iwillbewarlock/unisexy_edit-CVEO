# AI 작업 지침 참고자료 인덱스

모딩 작업 전반에서 "AI 에이전트가 어떻게 일해야 하는가"를 정의하기 위해 수집한 외부 자료 모음.
도구로 도입할지가 아니라 **문서 구조와 지침 패턴을 참고할 가치가 있는지**를 기준으로 평가한다.

평가 등급:
- **A** — 패턴을 직접 가져다 쓸 가치가 있음
- **B** — 일부 아이디어만 참고
- **C** — 우리 용도에 무관

## 요약

| # | 자료 | 정체 | 등급 | 상태 |
|---|------|------|:----:|------|
| 1 | [Nhahan/WebGPT](#1-nhahanwebgpt) | Codex 스킬 (ChatGPT 웹 위임) | **A** (문서만) | 검토 완료 |
| 2 | [Qkoa/alpha-ticket](#2-qkoaalpha-ticket) | Codex 스킬 (개발 워크플로 규약, 한국어) | **A** (패턴) | 검토 완료 |
| 3 | [ginishuh/contextforge](#3-ginishuhcontextforge) | — | — | 조사 중 |

---

## 1. Nhahan/WebGPT

<https://github.com/Nhahan/WebGPT>

| 항목 | 내용 |
|------|------|
| 정체 | **브라우저 LLM 추론이 아님.** "web ChatGPT"의 약자. 로그인된 ChatGPT 웹 UI에 작업을 위임하는 Codex CLI 스킬 |
| 목적 | Codex API 쿼터 절약 |
| 스택 | Node 22+ ESM, 유일 의존성 `node-pty` 1.1.0, cloudflared Quick Tunnel, MCP |
| 라이선스 | MIT |
| 활성도 | 별 43, 포크 10, 커밋 77개가 **거의 전부 2026-09-15 하루에 집중**. 이슈/PR 0. 1인 프로젝트 |
| 도구로서 | **C — 도입 안 함** |
| 문서로서 | **A — 참고 가치 있음** |

### 동작 방식

1. 로컬 Node 워커가 MCP 엔드포인트 개방 (포트 43137/43139)
2. cloudflared Quick Tunnel로 HTTPS 외부 노출
3. 해당 URL을 ChatGPT 웹 플러그인(커넥터)으로 등록 → 웹 ChatGPT가 `exec_command`로 내 터미널 조작
4. Codex가 접근성 트리(`getAXState`)로 ChatGPT 탭 조작, 프롬프트 전송
5. 결과 회수 후 해당 대화를 **영구 삭제**

모드: `webgpt xh` (Extra High, 기본) / `p` (Pro) / `open` (사용자 주도, 삭제 안 함)

### 도구로 도입하지 않는 이유

- **Codex 전용.** SKILL.md·setup.md가 전부 Codex 대상. 저장소 토픽의 `claude`/`claude-code`는 검색 유입용이며 연동 코드 없음
- **모딩의 병목이 LLM 쿼터가 아님.** 실제 병목은 MSVC/vcpkg 빌드 → 스카이림 실행 검증 루프인데 이 도구는 거기를 안 건드림
- **작업당 메시지 1개, 후속 불가.** 컴파일 에러 → 수정 → 재빌드 반복이 불가능. 모딩 작업 성격과 정면 충돌
- **컨텍스트 재공급 비용.** CommonLibSSE 헤더 + Address Library ID + AGENTS.md(15KB)를 매 채팅마다 다시 넣으면 절약분이 상쇄
- **보안.** 터널로 열린 URL에 OS 사용자 권한 full terminal access. 비밀 URL 하나가 유일한 관문. 문서가 "not a sandbox"를 반복 명시. 모딩 머신은 보통 스카이림 설치본·MO2 프로파일·Nexus 로그인이 있는 메인 데스크톱
- **UI 자동화 취약성.** ChatGPT DOM/모드 선택기/삭제 다이얼로그에 의존. 필터 문자열이 UI 언어에 종속
- Quick Tunnel URL은 재시작마다 변경됨
- 웹 UI 자동화의 약관 문제는 저장소에서 다루지 않음

### 가져올 문서 패턴

`skills/webgpt/SKILL.md` — 137줄. 형식이 Claude Code 스킬과 **동일** (YAML frontmatter `name` + `description` + SKILL.md).

**(1) 조건부 문서 로딩 — 가장 값어치 있음**
> "Read setup.md **only for** an installation request or an observed missing capability. Normal delegation reads workspace.md, not setup.md or the worker source."

메인 문서는 짧게, 무거운 참조 문서는 *언제 읽는지* 조건을 명시해 분리. 컨텍스트 낭비 방지의 핵심.

**(2) 금지 + 이유 + 대안을 세트로**
> "Do not call bare `getAXState()` or write the whole returned string: **either can reintroduce large automatic output.**"

바로 뒤에 올바른 호출 예시 1줄. 금지만 있으면 에이전트가 다른 방식으로 똑같이 틀린다.

**(3) LLM 실패 모드를 이름 붙여 금지**
> "Do not create separate tracking files unless recovery genuinely requires one."
> "Do not narrate unchanged waiting."
> "Do not independently redo WebGPT's investigation or implementation."

불필요한 파일 생성 / 과잉 보고 / 중복 작업.

**(4) 증거 규율 — 모딩에 그대로 적용 가능**
> "Distinguish PASS/FAIL/NOT_RUN"
> "**A completion claim alone is not evidence.**"
> "Preserve partial results and report blockers honestly."

**(5) 책임 소재 명시**
> "**The persistent worker, not Codex,** handles expiry."

누가 뭘 담당하는지 박아두면 에이전트가 이미 있는 걸 다시 구현하지 않는다.

**(6) 생애주기 단계 분리** — Open / Dispatch / Collect / Close

### 베끼지 말 것

- 산문 밀도가 과함. 137줄에 규칙이 빽빽해 사람이 유지보수하기 어려움 → 목록으로 풀어쓸 것
- 자기 도구 전용 지식(`getAXState`, `nodeRepl`, `sendOnce`)이 일반 규칙과 섞여 있음 → 분리 필요
- **검증된 물건이 아님.** 1주일 된 1인 프로젝트라 "이 패턴이 실제로 잘 먹힌다"는 근거 없음. 구조만 참고

---

## 2. Qkoa/alpha-ticket

<https://github.com/Qkoa/alpha-ticket>

| 항목 | 내용 |
|------|------|
| 정체 | **티켓 판매 서비스가 아님.** 장기 실행 개발 워크플로를 정의한 Codex Agent Skill. 저장소 내용물이 사실상 전부 에이전트 지침 문서 |
| 언어 | **산문이 전부 한국어** (식별자/표 헤더만 영어). 트리거도 한글: `알파티켓`, `설계 승인`, `커밋 승인`, `패치루프`, `티켓 클로즈` |
| 스택 | Markdown 위주 + Python 3.11+ 표준 라이브러리만. 외부 패키지·MCP 서버 불필요 |
| 라이선스 | MIT |
| 활성도 | **별 3, 포크 2, 커밋 2개, 1인 저자.** 이슈/PR 0, CI 없음. 매우 미검증 |
| 등급 | **A (패턴) / C (전량 도입)** |

### 구조

`alpha-ticket/` 이 설치 대상 스킬. `SKILL.md`(7KB, 한국어 7개 절) + `references/`(design, implement-verify, review-close, operations, migration) + `templates/`(10종) + `scripts/alpha_lint.py`.

워크플로: 명시적 트리거로만 발동 → `index.md` 표를 grep해 **작업 범위만큼만** 컨텍스트 조립 → 설계(A~D절) 작성 후 **정지, 승인 대기** → 구현·검증·구조 감사 → **정지, 커밋 승인 대기** → 커밋/푸시/PR/리뷰 → 클로즈. 클로즈된 작업 단위가 5의 배수가 될 때마다 최근 5개에 대해 "의미 감사" 수행.

### 가져올 패턴

**(1) 승인 게이트 — 모딩에 즉시 적용 가능**
> "설계 승인 전 일반 알파티켓 구현을 시작하지 않는다."
> "커밋 승인 전 commit·push·PR·리뷰 요청을 하지 않는다."

되돌리기 어려운/파괴적/비용 발생/보안 경계 변경은 별도 확인. 무관한 dirty·untracked 작업은 보존하고 승인된 diff에서 제외.

**(2) 모드별 검증 폭 — Fast / Core / Strict**

최소 검증 범위를 모드에 매핑한다: focused(해당 모듈만) / related(역방향 `uses` 소비자까지) / full(전체 회귀). 모딩판으로 번역하면 **컴파일만 / 컴파일 + 관련 기능 / 컴파일 + 인게임 검증**. 리뷰 강도도 같이 매핑(Fast: 없음, Core: 리뷰어 1, Strict: 리뷰어 1 + 적대적 조건).

**(3) 린터로 기계 검증 — 이 저장소의 최대 아이디어**

`alpha_lint.py`가 에이전트가 쓴 문서를 read-only로 검사해 구조화된 findings(level/code/message/path/ticket)를 JSON으로 뱉는다. 에러와 레거시 호환 경고를 구분. **지침을 글로만 두지 않고 기계가 강제한다**는 발상이 핵심.

**(4) 정확 매칭 규율 — FormID/Address ID에 그대로 적용됨**

`WU-1` 검색이 `WU-10`에 부분 매칭되면 안 되므로 셀 구분자까지 포함해 `| WU-1 |`로 검색하라고 명시. 우리 쪽 FormID·Address Library ID 취급에 똑같은 함정이 있다.

**(5) 진행적 공개 라우터** — 7절이 "설계면 design.md, 구현·검증이면 implement-verify.md, 커밋·리뷰·클로즈면 review-close.md" 식 분기. 마지막에 **"프로젝트 고유 사실은 스킬이 아니라 소비 저장소의 PRD/WORK/INDEX/Memory에 둔다"** — 재사용 규약과 프로젝트 지식의 분리를 명문화.

**(6) 안티-블로트 규칙** — 별도 Decisions 문서, "Resume Packet", 세션 체크포인트 블록, 세션 인계 요약을 만들지 말 것.

**(7) 템플릿 = 예시가 아니라 정본 스키마** — 새 문서는 front matter와 표 컬럼을 그대로 복사. 형식 표류 방지.

### 주의점 / 베끼지 말 것

- **의식(ceremony)이 과도함.** `prd.md`, `work/WU-*.md`, `ir/`, `ol/`, `rft/`, `queue.md`, `index.md`, `memory.md` 문서 체계 + 4계층 아키텍처(Foundation/Platform/Feature/Workflow) + 의존 허용표. **소스 11개짜리 SKSE 플러그인 하나에 씌우기엔 명백히 과함.** 게이트와 검증 폭만 떼어 오는 게 맞다
- **극도로 미검증.** 별 3개, 커밋 2개, 1인 저자, CI 없음
- Codex 종속 부분 존재: `$skill-installer` 설치, `@codex review`를 마지막 GitHub 쓰기로 남기라는 규칙, `agents/openai.yaml`
- 버전 표기 불일치: README는 v3.2.3인데 초기 커밋은 v3.2.1, 마이그레이션 파일명도 `migration-v3.2.1.md`
- 린터 자체 품질은 단일 파일치고 양호하나(dataclass, 현대적 타이핑, 파싱 실패를 크래시 대신 기록) `header_findings()`/`check_doc()` 약 40줄 중복 등 존재. 테스트는 픽스처 e2e만 있고 헬퍼 단위 테스트 없음

---

## 3. ginishuh/contextforge

<https://github.com/ginishuh/contextforge>

조사 중.

---

## 현재 저장소에 대한 관찰

인덱스 작업 중 확인된 사항 (자료와 별개로 기록).

1. **`AGENTS.md`가 Claude 세션에서 자동 로드되지 않음.** 이번 세션에서 컨텍스트에 주입되지 않아 직접 열어야 했다. `CLAUDE.md`도 `.claude/` 디렉터리도 없음. 공들여 쓴 15KB 문서가 놀고 있을 가능성이 큼

2. **지식 문서와 행동 규약이 한 파일에 혼재.** `AGENTS.md`는 "이 프로젝트는 이렇게 생겼다"(지식)인데, 7장 "빌드 실패 시 참고 (과거 사례)"는 경계에 걸쳐 있음 — 사례 나열이지 규칙이 아니라 같은 실수 반복 여지가 있고, 빌드가 안 깨졌을 때도 매번 전부 로드됨. WebGPT 패턴 (1)이 정확히 이걸 푸는 기법

3. **CI 없음.** `.github/` 자체가 부재. 빌드 검증이 전적으로 로컬 수동. `windows-latest` + vcpkg로 SE/AE 프리셋을 빌드하는 워크플로가 있으면 에이전트 수정분이 푸시 즉시 컴파일 검증됨

4. **재사용 관점.** 지식 문서는 모드마다 새로 써야 하지만, 행동 규약(빌드 검증 규칙, Address Library ID 취급법, 증거 보고 형식)은 모드가 늘어나도 그대로 재사용됨 → 분리의 실익이 큼
