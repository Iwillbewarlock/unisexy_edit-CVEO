# 에이전트 작업 규약 — 출처 기록

> **정본은 `AGENTS.md` 0장이다.** 이 파일은 각 규칙이 어디서 왔는지의 기록만 남긴다.
> 규칙을 고칠 때는 `AGENTS.md`를 고치고, 근거가 바뀌면 여기를 같이 고친다.
> 다른 모드 저장소로 가져갈 때는 `AGENTS.md` 0.1~0.7을 복사한다. 0.8은 그 저장소 상황에 맞게 다시 쓴다.

## 출처

수렴 = `docs/ai-refs/INDEX.md`의 패턴 수렴 절에서 2개 이상 자료가 독립적으로 도달한 것.

| 규칙 | 출처 |
|---|---|
| R-1.1, R-1.5 | 현행 `AGENTS.md` 0장 |
| R-1.2~1.4 | contextforge 한국어 응대 원칙 |
| R-1.6 | 사용자 요청 (2026-09-22). 비개발자 대상 설명 규칙 |
| R-2.1, R-2.2, R-2.5 | 수렴(증거 규율). WebGPT "completion claim alone is not evidence", ui-ux "never present a 0-result search as if it returned data" |
| R-2.3 | 현행 `AGENTS.md` 0장 + 8장 디버깅 노트 |
| R-3 표 | alpha-ticket Fast/Core/Strict를 모딩판으로 번역 |
| R-3.3 | Claude Code 하네스 규칙 (테스트 비활성화 금지) |
| R-4.1 | contextforge "agents may create and update PRs but must not merge them" |
| R-4.2 | 수렴(승인 게이트). alpha-ticket 커밋 승인 + 현행 `AGENTS.md`의 "되돌리지 말 것"·"근거 없이 되살리지 말 것" |
| R-4.4 | alpha-ticket "무관한 dirty·untracked 작업 보존" |
| R-4.5 | contextforge 의도적 non-DRY 기록 + 2026-08-18 오진 사례 |
| R-5.1, R-5.2 | alpha-ticket 정확 매칭 규율 (`WU-1` vs `WU-10`) |
| R-5.3 | ui-ux "never assume a stack" |
| R-5.4 | ui-ux "do not install it yourself" |
| R-5.5 | ui-ux "recommendations, never instructions" + 하네스의 서브에이전트 보고 취급 규칙 |
| R-5.6 | ui-ux "retry once, then label fallback, do not persist unverified" |
| R-6.1 | 수렴 3/3 (진행적 공개) |
| R-6.2, R-6.3 | 수렴 3/3 (안티-블로트). WebGPT "do not narrate unchanged waiting" |
| R-6.5 | WebGPT 금지+이유+대안 패턴 |
| R-7.2, R-7.3, R-7.4 | 현행 `AGENTS.md` 3장·4장·1장의 사고 이력을 규칙화 |
| R-7.5 | contextforge 의도적 non-DRY |
| 8절 | 수렴 2/3 (기계 강제). alpha-ticket 린터, contextforge 라인 예산 래칫, claude-code-setup `hooks-patterns.md` |
