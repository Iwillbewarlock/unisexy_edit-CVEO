# AGENTS.md — Unisexy-SKSE

이 파일은 AI 에이전트(Claude, Antigravity, Copilot 등)가 이 저장소에서 작업할 때 먼저 읽는 지침서다.
사람이 읽어도 무방한 프로젝트 현황 요약을 겸한다.

## 0. 기본 규칙

- 사용자와의 대화는 **한국어**로 한다. 코드 주석·커밋 메시지·로그 문자열은 **영어**로 쓴다.
- 코드 스타일은 저장소의 `.clang-format`을 따른다 (탭 들여쓰기 4, `ColumnLimit: 0`, C++23).
- `extern/` 아래는 서드파티(CommonLibSSE 서브모듈, 벤더링된 clib-util)다. **직접 수정하지 않는다.**
- 빌드 산출물(`build/`, `buildae/`, `vcpkg_installed/`)은 `.gitignore` 대상이다. 커밋하지 않는다.
- 게임 런타임을 실제로 실행해 검증할 수 없는 환경이 대부분이다. 컴파일 성공까지가 자동 검증의 한계이며,
  런타임 동작(헤드파트 생성, FormID 충돌)은 사용자에게 SKSE 로그 확인을 요청한다.

## 1. 프로젝트 개요

Skyrim SE/AE용 SKSE 플러그인. 게임 로드 시점에 헤드파트(머리카락, 눈, 흉터, 눈썹, 수염)를
**성별 반전 복제본으로 동적 생성**해서, 남성 캐릭터가 여성 전용 헤어를 쓰는 식의 사용을 가능하게 한다.
ESP/ESM 파일을 새로 만들지 않고 런타임에 폼을 생성·등록하는 방식이라 FormID 할당이 핵심 로직이다.

- 저장소: `https://github.com/Iwillbewarlock/Unisexy-SKSE.git` (Fuzzlesz/Unisexy-SKSE 포크)
- 현재 버전: **2.1.0** (`CMakeLists.txt`의 `VERSION`. `vcpkg.json`은 아직 2.0.3 — 불일치, 아래 6절 참조)
- 라이선스: MIT

## 2. 빌드

### 요구 환경 (환경변수)

| 변수 | 용도 |
|---|---|
| `VCPKG_ROOT` | vcpkg 루트 경로 |
| `CommonLibSSEPath` | CommonLibSSE 경로. `extern/CommonLibSSE`에 서브모듈이 있으면 그쪽이 우선 |
| `Skyrim64Path` / `SkyrimAEPath` | (선택) `COPY_BUILD=ON`일 때 산출물 복사 위치 |

Visual Studio 2022 (Desktop development with C++), CMake 3.21+, PowerShell 필요.

### 명령

```bash
git submodule init && git submodule update   # extern/CommonLibSSE 필수

# SSE
cmake --preset vs2022-windows-vcpkg-se
cmake --build build --config Release

# AE
cmake --preset vs2022-windows-vcpkg-ae
cmake --build buildae --config Release
```

산출물: `build/Release/Unisexy.dll` → `Data/SKSE/Plugins/`에 배치.

### CommonLib 주의 (중요)

이 프로젝트는 **powerof3/CommonLibSSE (dev 브랜치)** 를 쓴다. CommonLibSSE-**NG**가 아니다.
과거에 `FetchContent`로 CommonLibSSE-NG를 받아오던 구성이었으나 2.1.0에서 제거했다.
NG에는 `TESForm::SetFile()`이 없어 별도 워크어라운드가 필요했고, 그 코드도 함께 제거됐다.
**NG로 되돌리지 말 것.** 되돌리면 `SetFile` 관련 컴파일 에러가 재발한다.

`CMakeLists.txt`는 `SKYRIM_AE` / `SKYRIM_SUPPORT_AE`를 **SE 프리셋에서도 항상** 정의한다.
따라서 `main.cpp`의 `#ifdef SKYRIM_AE` 분기는 실질적으로 항상 참이고,
플러그인은 `SKSEPlugin_Version`(SKSE 2.2.6+ 방식)만 내보낸다. `SKSEPlugin_Query`는 죽은 코드 경로다.

## 3. 코드 구조 (`src/`)

| 파일 | 역할 |
|---|---|
| `main.cpp` | SKSE 진입점. 버전 데이터 export, 로그 초기화, `kDataLoaded` 메시지에서 `Unisexy::DoSexyStuff()` 호출 |
| `Unisexy.cpp/.h` | 메인 로직. 전체 헤드파트 순회 → 성별 판정 → 복제본 생성 → DataHandler 등록 → 통계 로깅 |
| `HeadPartUtils.cpp/.h` | EditorID 생성, 헤드파트 복제, `extraParts`(서브메시) 재귀 처리, CVEO 판별 |
| `Settings.cpp/.h` | `Data/SKSE/Plugins/Unisexy.ini` 로드/마이그레이션/저장. 구 포맷 키 자동 변환 |
| `FormIDManager.cpp/.h` | EditorID 해시 기반 FormID 할당 + 충돌 회피. ESL(`0xFE` 플래그)/ESP 양쪽 지원 |
| `PCH.h`, `PCH.cpp` | 미리 컴파일된 헤더. CommonLibSSE, spdlog, CLIBUtil 포함 |

파일을 추가하면 `cmake/sourcelist.cmake` / `cmake/headerlist.cmake`에 **반드시 등록**해야 한다.

## 4. 설정 파일 (`Data/SKSE/Plugins/Unisexy.ini`)

```ini
[HeadPartTypes]
HairMale / ScarsMale / EyesMale / BrowsMale / FacialHairMale     ; 여성 파트 → 남성 버전 생성
HairFemale / ScarsFemale / EyesFemale / BrowsFemale / FacialHairFemale  ; 남성 파트 → 여성 버전 생성

[Debug]
VerboseLogging    ; 상세 로깅
ShowOnlyUnisexy   ; 원본 파트 숨기고 Unisexy 버전만 노출 (전역)

[ShowOnlyUnisexy]  ; 2.1.0 신규 — 카테고리별 개별 제어
ShowOnlyUnisexyHair / ShowOnlyUnisexyEyes / ShowOnlyUnisexyScars
ShowOnlyUnisexyBrows / ShowOnlyUnisexyFacialHair
```

기본값은 Hair와 Eyes만 `true`. `Settings::Load()`가 구 포맷(`Hair`, `Eyes`, `DisableVanillaParts` 등
성별 구분 없는 키)을 감지하면 자동 마이그레이션 후 ini를 다시 저장한다.

## 5. 최근 작업 내역 (2.1.0, 커밋 `eef0f2e`)

커밋 메시지: *"Fix SKSE 2.2.6 AE export, FormID exhaustion fallback, SimpleIni assertion, and extraParts binding"*

1. **SKSE 2.2.6 AE export** — `main.cpp`를 `#ifdef SKYRIM_AE`로 분기, `SKSEPlugin_Version` 사용.
   CMake에서 AE 정의를 항상 켜도록 변경.
2. **FormID 고갈 폴백** — `FormIDManager::AssignFormID`에 3단 폴백 추가:
   해시 기반 재시도 → 카운터에서 하향 스캔 → 원래 베이스에서 상향 스캔. 전부 실패해야 error.
   `GetFileFromFormID()`와 미사용 `formCounts_` 멤버는 삭제.
3. **assert 제거** — `assert()`로 죽던 널 체크를 `logger::error` + early return으로 교체.
   Release 빌드에서 assert가 무력화되던 문제 대응.
4. **extraParts 바인딩** — 기존 파트 재사용 시 전체 폼 배열을 선형 검색하던 것을
   `std::unordered_map<std::string, RE::BGSHeadPart*>` 룩업으로 교체 (성능).
   생성된 서브메시 파트에 `kPlayable` 플래그를 강제로 세팅.
5. **CVEO 대응** — Complete Vanilla Eye Overhaul의 커스텀 슬라이더 타입(171~193)을
   `kEyes` 카테고리로 매핑. `IsCVEOFile()` / `IsCVEOSliderType()` 추가.
6. **유니섹스 파트 처리 변경** — 남녀 플래그가 둘 다 있거나 둘 다 없는 파트는
   더 이상 비활성화하지 않고 그냥 건너뛴다.
7. **CMake 정리** — CommonLibSSE-NG FetchContent 제거, `find_commonlib_path()` 매크로 도입,
   `find_path(CLIB_UTIL_INCLUDE_DIRS ...)`로 clib-util 경로 탐색.
8. `PCH.h`의 템플릿 함수에 `inline` 추가 (중복 정의 방지).

**현재 상태: 빌드 성공.** `build_log6~8.txt`가 에러 없이 `Unisexy.dll` 생성으로 끝난다.
작업 트리는 `eef0f2e`와 동일하고 origin에 푸시된 상태다. 미커밋 변경 없음.

## 6. 알려진 이슈 / 다음에 할 일

정리 완료 (2026-08-18):

- `vcpkg.json`의 `version-string`을 `2.1.0`으로 맞춰 `CMakeLists.txt`와 동기화했다.
- 배포용 ini(`Data/SKSE/Plugins/Unisexy.ini`)에 `[ShowOnlyUnisexy]` 섹션과 눈 관련 주석을 추가했다.
  키 순서도 코드(`Settings::SaveConfigFile`)가 쓰는 순서(Hair → Scars → Eyes → Brows → FacialHair)에 맞췄다.
- `Skyrim/Data/SKSE/plugins/Unisexy.ini`를 위 파일과 동일한 내용으로 동기화했다.
- `.gitignore`에 `build_log*.txt`, `cmake_log.txt`, `cmake_output.txt`, `large_files.txt`를 추가했다.

남은 일:

- **로그 파일은 아직 git이 추적 중이다.** `.gitignore` 추가만으로는 빠지지 않으므로 아래를 실행해야 한다:
  ```
  git rm --cached build_log*.txt cmake_log.txt cmake_output.txt large_files.txt
  git commit -m "Untrack build logs"
  ```
  `build_log.txt`는 15MB라 이력에서 완전히 지우려면 `git filter-repo` 등 별도 작업이 필요하다.
  (작업 트리의 파일 자체는 남겨두었다. 필요 없으면 직접 삭제.)
- **ini 사본이 여전히 2벌이다.** 내용은 같게 맞췄지만 `Skyrim/Data/...` 쪽은 용도가 불분명하다.
  불필요하면 `git rm -r Skyrim`으로 제거하는 편이 유지보수에 낫다.
- **clib-util 경로 대소문자 불일치**: 벤더링된 디렉터리는 `extern/clib-util/include/CLIBUtil/`인데
  코드와 CMake는 `ClibUtil/...`로 참조한다. Windows에서는 문제없지만 대소문자 구분 환경에서는 깨진다.
- 런타임 검증 미완: CVEO 눈 슬라이더 매핑과 FormID 상향 폴백은 실제 게임에서 확인되지 않았다.
- 위 변경 이후 재빌드는 아직 하지 않았다 (ini/gitignore/vcpkg 메타데이터만 건드려 컴파일에는 영향 없음).

## 7. 빌드 실패 시 참고 (과거 사례)

| 증상 | 원인 |
|---|---|
| `rapidcsv.h: No such file` | vcpkg 의존성 미설치 / `VCPKG_ROOT` 미설정 |
| `ClibUtil/singleton.hpp: No such file`, `std::uint32_t가 std의 멤버가 아님`, `logger 네임스페이스 아님` | PCH 미적용 또는 clib-util include 경로 누락 |
| `CLIB_UTIL_INCLUDE_DIRS ... NOTFOUND` | `cmake/ports/clib-util` 오버레이 포트가 vcpkg에 안 잡힘 |
| `'SetFile': RE::BGSHeadPart의 멤버가 아님` | CommonLibSSE-**NG**를 쓰고 있음. powerof3 버전으로 교체 |
| `'RUNTIME_1_5_39': SKSE의 멤버가 아님` | SKSE 2.2.6 이상. `SKSEPlugin_Query` 경로를 쓰고 있음 |
| `warning D9025: '/Ob2'를 '/Ob3'로 재정의` | 무해. 무시해도 됨 |
