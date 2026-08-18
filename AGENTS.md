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
  - 리모트가 둘이다. `origin` = 위 주소, `cveo` = `https://github.com/Iwillbewarlock/unisexy_edit-CVEO.git`
    (CVEO 눈 대응 작업 라인). 푸시 대상을 착각하지 말 것.
- 현재 버전: **2.1.0** (`CMakeLists.txt`, `vcpkg.json` 동기화 완료)
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

### VS2022가 없는 환경

`CMakePresets.json`의 프리셋은 `Visual Studio 17 2022` 제너레이터와 `v143` 툴셋을 못박아 둔다.
VS2026 등 다른 버전만 설치돼 있으면 configure가 `could not find any instance of Visual Studio`로 죽는다.
이때는 `CMakeUserPresets.json`(gitignore 대상)을 만들어 제너레이터만 바꾼 프리셋을 추가한다.
**툴셋은 지정하지 말 것** — `v143`을 남겨두면 `MSB8020`으로 다시 실패한다.

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "vs2026-windows-vcpkg-se",
      "inherits": ["cmake-dev", "vcpkg", "windows", "se"],
      "generator": "Visual Studio 18 2026"
    }
  ]
}
```

### clib-util 의존성 (중요)

`vcpkg.json`의 `clib-util`은 **반드시 있어야 한다.** 이게 빠지면 `cmake/ports/clib-util` 오버레이 포트가
설치되지 않아 `find_path(CLIB_UTIL_INCLUDE_DIRS ...)`가 NOTFOUND로 떨어지고 generate 단계에서 죽는다.
과거 커밋(`85cd0f9`)에서 실수로 제거됐다가 `a17b7a0`에서 복구됐다. 참고로 `rapidcsv`는
CMakeLists 어디서도 참조하지 않는다(제거 가능).

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

기본값(`a17b7a0` 기준):

- `[HeadPartTypes]` — **전 카테고리 양방향 `true`**
- `[ShowOnlyUnisexy]` — **머리카락만 `false`, 나머지 넷은 `true`**
- `[Debug]` — 둘 다 `false`

기본값은 **세 군데가 항상 일치해야 한다.** 하나라도 어긋나면 "설정한 대로 안 된다"는 증상이 나온다:

1. `Settings::Load()` 상단의 `_enabledTypes` / `_showOnlyUnisexyTypes` 하드코딩 초기값
2. 각 `ini.GetBoolValue(..., <기본값>, ...)`의 폴백 인자
3. 배포 템플릿 `Data/SKSE/Plugins/Unisexy.ini` 및 `Skyrim/Data/SKSE/plugins/Unisexy.ini` 사본

실제로 2026-08-18에 이 불일치로 시간을 크게 날렸다. 템플릿엔 `EyesMale=true`인데 코드 기본값은
`{false, false}`였고, MO2 환경에서는 모드 폴더에 ini가 없어 **코드 기본값으로 새로 생성**되기 때문에
템플릿은 아무 영향도 없었다. 결과적으로 CVEO 눈이 한 개도 변환되지 않았다.

`Settings::Load()`가 구 포맷(`Hair`, `Eyes`, `DisableVanillaParts` 등 성별 구분 없는 키)을 감지하면
자동 마이그레이션 후 ini를 다시 저장한다.

### ShowOnlyUnisexy의 실제 동작 (헷갈리기 쉬움)

원본을 숨기는 코드는 `Unisexy.cpp` 루프의 **맨 끝**, 복제본 생성이 성공한 뒤에만 실행된다. 따라서:

- 변환이 꺼진 카테고리는 `ShowOnlyUnisexyXxx`를 켜도 **아무 일도 일어나지 않는다.**
  전역 `[Debug] ShowOnlyUnisexy=true`도 마찬가지다. 원본만 사라지는 사고는 구조상 발생하지 않는다.
- 방향이 갈린다. 여성 전용 파트는 `<Category>Male=true`여야, 남성 전용 파트는 `<Category>Female=true`여야
  숨겨진다. 한 카테고리의 원본을 완전히 치우려면 **양방향 모두** 켜야 한다.
- FormID 할당 실패 등으로 복제에 실패한 파트도 `continue`로 빠져 원본이 그대로 남는다(안전장치).

## 5. 작업 내역 (2.1.0, 커밋 `eef0f2e`, 2026-08-10)

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

## 5-1. 작업 내역 (커밋 `a17b7a0`, 2026-08-19) — 가장 최근

커밋 메시지: *"Fix build, widen ESL FormID range, and enable all conversions by default"*

**빌드 복구** — 이 커밋 이전 트리는 컴파일도 configure도 안 됐다.

1. `main.cpp`가 존재하지 않는 `SKSE::RUNTIME_SSE_LATEST`를 참조 → `SKSE::RUNTIME_LATEST`로 수정.
   powerof3/CommonLibSSE의 `SKSE/Version.h`에는 `RUNTIME_LATEST`만 있다.
2. `vcpkg.json`에 `clib-util` 복구 (위 2절 참조).
3. `.gitignore`에 `CMakeUserPresets.json` 추가.

**ESL FormID 고갈 해소** — 이번 커밋의 핵심.

`FORMID_MIN = 0x800` 상수 하나를 ESL/ESP가 공유하고 있었다. 라이트 플러그인은 FormID 공간이
12비트(0x000~0xFFF)뿐인데 하한을 0x800으로 잡아 **가용 공간의 절반을 버리고 있었다.**
KS Hairdo's, CVEO 눈 애드온 같은 대형 ESL에서 슬롯이 고갈돼 파트가 대량으로 누락됐다
(한 번은 415건 실패). `ESL_LOW_START(0x001)` / `ESP_LOW_START(0x800)`으로 분리하고
`GetFormIDMin(isLight)`로 감쌌다. Skyrim 1.6.1130+는 ESL 전 범위를 허용하므로
가용 슬롯이 **2048 → 4095**로 늘었다. ESP/ESM 동작은 그대로다.

**설정 기본값 정비** — 4절 참조. 전 카테고리 변환 활성, ShowOnlyUnisexy는 머리 제외 활성,
폴백 인자까지 전부 일치시켰다.

**ini 설명문 확충** — 플레이어가 소스를 안 봐도 각 스위치를 이해할 수 있게 주석을 채웠다.
이전 주석에 있던 "변환이 꺼진 카테고리는 원본만 사라진다"는 **사실이 아니어서** 삭제했다(4절 참조).

**현재 상태: SE/AE 양쪽 Release 빌드 성공, 게임에서 정상 작동 확인됨(2026-08-19).**

## 6. 알려진 이슈 / 다음에 할 일

해결됨:

- 빌드 로그 파일 git 추적 제거 (`b23a2c2`), `vcpkg.json` 버전 동기화 (`b23a2c2`).
- 컴파일/configure 실패 2건, ESL FormID 고갈, 설정 기본값 불일치 (`a17b7a0`, 5-1절 참조).

남은 일:

- **CVEO 헬퍼가 죽은 코드다.** `HeadPartUtils::IsCVEOFile()` / `IsCVEOSliderType()`는 정의·선언만
  있고 **호출하는 곳이 한 군데도 없다.** `eef0f2e`가 CVEO 전용 분기를 지우고 "슬라이더 타입 171~193을
  `kEyes`로 매핑"하는 방식으로 대체하면서 남은 잔해다. 현재 CVEO 눈은 일반 `kEyes` 경로로 처리되며
  `EyesMale`/`EyesFemale` 설정을 따른다(기본 `true`라 동작함). 헬퍼를 지우든 다시 연결하든 정리 필요.
- **`backup_20260810_022644` 폴더의 `validRaces` 실험본에 주의.** `Unisexy-SKSE-edit` 작업 폴더에
  `CreateUnisexyHeadPart()`에서 성별에 맞춰 `validRaces`를 교체하는 33줄 블록이 있는 백업이 있다.
  이 저장소 어느 커밋에도 없고, 실제로 동작 확인된 DLL에도 들어있지 않다. 버려진 방향으로 보이므로
  근거 없이 되살리지 말 것. (2026-08-18에 이걸 원인으로 오진해서 시간을 날렸다. 진짜 원인은 ini였다.)
- **ini 사본이 2벌이다.** `Data/SKSE/Plugins/`와 `Skyrim/Data/SKSE/plugins/`. 내용은 동일하게
  유지 중이지만 후자의 용도가 불분명하다. 불필요하면 `git rm -r Skyrim`.
- **clib-util 경로 대소문자 불일치**: 벤더링된 디렉터리는 `extern/clib-util/include/CLIBUtil/`인데
  코드와 CMake는 `ClibUtil/...`로 참조한다. Windows에서는 문제없지만 대소문자 구분 환경에서는 깨진다.
- **로그 배너가 `Unisexy v2-0-0-0`으로 찍힌다.** `Version.h`는 `NAME = "2.1.0"`이고 빌드된 DLL을
  바이너리 검색해도 `2-0-0-0` 문자열이 없는데 로그에는 나온다. 배너 줄과 처리 줄의 스레드 ID가 다르고
  시각도 수 분 차이가 나서, 다른 프로세스가 같은 로그 파일에 쓰는 것으로 추정만 하고 있다.
  **기능에는 영향 없음.** 원인 미확인.
- `MAX_FORMID_ATTEMPTS = 10` 이후의 하향/상향 스캔 폴백은 실제 게임에서 발동 여부가 확인되지 않았다.

## 7. 빌드 실패 시 참고 (과거 사례)

| 증상 | 원인 |
|---|---|
| `rapidcsv.h: No such file` | vcpkg 의존성 미설치 / `VCPKG_ROOT` 미설정 |
| `ClibUtil/singleton.hpp: No such file`, `std::uint32_t가 std의 멤버가 아님`, `logger 네임스페이스 아님` | PCH 미적용 또는 clib-util include 경로 누락 |
| `CLIB_UTIL_INCLUDE_DIRS ... NOTFOUND` | `cmake/ports/clib-util` 오버레이 포트가 vcpkg에 안 잡힘 |
| `'SetFile': RE::BGSHeadPart의 멤버가 아님` | CommonLibSSE-**NG**를 쓰고 있음. powerof3 버전으로 교체 |
| `'RUNTIME_1_5_39': SKSE의 멤버가 아님` | SKSE 2.2.6 이상. `SKSEPlugin_Query` 경로를 쓰고 있음 |
| `'RUNTIME_SSE_LATEST': SKSE의 멤버가 아님` | 그런 심볼은 없다. `SKSE::RUNTIME_LATEST`를 쓸 것 |
| `CLIB_UTIL_INCLUDE_DIRS ... NOTFOUND` (generate 단계) | `vcpkg.json`에서 `clib-util`이 빠졌다. 2절 참조 |
| `could not find any instance of Visual Studio` | VS2022 미설치. `CMakeUserPresets.json`으로 제너레이터 교체. 2절 참조 |
| `MSB8020: v143 빌드 도구를 찾을 수 없습니다` | 위 사용자 프리셋에 `"toolset": "v143"`을 남겨둔 경우. 툴셋 지정을 빼면 된다 |
| `warning D9025: '/Ob2'를 '/Ob3'로 재정의` | 무해. 무시해도 됨 |

## 8. 디버깅 노트

- **MO2 환경에서 ini는 `overwrite`에 생성된다.** 모드 폴더에 ini를 넣어두지 않으면 코드 기본값으로
  새로 만들어지고, 배포 템플릿은 아무 역할도 하지 못한다. "설정한 대로 안 된다" 싶으면
  **소스를 파기 전에 `overwrite/SKSE/Plugins/Unisexy.ini`부터 확인할 것.**
- 게임 런타임 검증이 필요한 변경은 `Documents/My Games/Skyrim Special Edition/SKSE/Unisexy.log`를 본다.
  요약 줄(`Processing completed ... created N new parts, disabled M original parts`)의 N/M 변화가
  가장 빠른 판정 수단이다.
- 어떤 소스로 빌드된 DLL인지 확인해야 할 때는 바이너리에서 특징적인 로그 문자열을 찾으면 된다
  (예: `no available FormIDs remain`은 `eef0f2e` 이후에만 존재). 해시 비교와 병행하면 확실하다.
