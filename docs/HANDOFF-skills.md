# Handoff: Claude Code 범용 스킬 세팅

클라우드 세션에서 정리한 내용을 로컬/다른 세션으로 넘기기 위한 인계 문서.

## 결론 (추천 스킬)

| 우선순위 | 스킬 | 이유 |
|---|---|---|
| 1 | superpowers (obra/superpowers) | 계획→구현→디버깅 절차 강제. 자동 트리거. 범용 |
| 2 | find-skills (vercel-labs/skills) | 필요할 때 스킬 검색·설치 (`npx skills find`) |
| 3 | codex (openai/codex-plugin-cc) | 교차 모델 리뷰. ChatGPT 계정 있을 때만 |

건너뛸 것: gstack(superpowers와 중복), caveman(한국어 출력과 안 맞음),
claude-hud(터미널 전용, 앱에선 안 보임), 디자인/마케팅 계열(필요할 때만).

이미 내장된 것(중복 설치 불필요): /code-review, /security-review, /simplify,
skill-creator, deep-research, /loop

## 로컬 설치 (PC에서 1회)

`claude` 실행 후:
```
/plugin marketplace add obra/superpowers-marketplace
/plugin install superpowers@superpowers-marketplace
```
PowerShell:
```
npx skills add vercel-labs/skills     # find-skills 선택, global 범위
```
(선택) codex:
```
/plugin marketplace add openai/codex-plugin-cc
/plugin install codex@openai-codex
/reload-plugins
/codex:setup
```
확인: `claude` 안에서 `/plugin`.

## 폰 앱 연결

PC 작업 폴더에서 `claude remote-control` 실행 후 창 유지 →
폰 Claude 앱 Code 목록에 표시. 같은 계정 로그인 필요. PC 꺼지면 끊김.

## 남은 할 일 / 주의

- 클라우드 세션은 설치가 유지되지 않음. 클라우드에서도 쓰려면 환경 Setup script에
  설치 명령을 넣거나 저장소 `.claude/settings.json`에 플러그인 등록.
- 개인 스킬 `skse-plugin-scaffold`는 CommonLibSSE-NG 기준. 이 저장소는
  powerof3 CommonLibSSE 사용(AGENTS.md: NG로 되돌리지 말 것). 이 저장소에서 쓸 때 주의.
- 제안했으나 미착수: 이 저장소 전용 스킬(ini 기본값 3곳 일치 검사, sourcelist 등록 검사).
