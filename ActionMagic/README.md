# ActionMagic

스카이림 SE/AE용 SKSE 플러그인. 마법을 액션 게임처럼 쓰기 위한 실험 프로젝트다.
설계와 단계별 계획은 [`DESIGN.md`](DESIGN.md)를 본다.

> 이 디렉터리는 상위 저장소의 Unisexy 플러그인과 **별개의 프로젝트**다. 빌드도 따로 한다.
> CommonLibSSE는 상위 저장소의 `extern/CommonLibSSE` 서브모듈을 함께 쓴다.

## 현재 기능 (0.1.0, 1단계)

지팡이를 들지 않은 상태에서 **왼쪽 Shift + 마우스 휠**로 마법 즐겨찾기에 등록한 주문을 순서대로 손에 장착한다.

- 휠 아래 = 다음 주문, 휠 위 = 이전 주문
- 기본은 오른손. 오른손에 무기·방패·횃불을 들고 있으면 왼손에 장착한다.
- 즐겨찾기의 권능(power)과 용언(shout)은 건너뛴다.
- 지팡이를 들고 있으면 아직 동작하지 않는다 (2단계 예정).

## 빌드

요구 사항은 상위 저장소와 같다: Visual Studio 2022, CMake 3.21+, vcpkg(`VCPKG_ROOT`).

```bash
# 저장소 루트에서 CommonLibSSE 서브모듈 준비
git submodule update --init extern/CommonLibSSE

cd ActionMagic
cmake --preset vs2022-windows-vcpkg
cmake --build build --config Release
```

산출물: `build/Release/ActionMagic.dll` → `Data/SKSE/Plugins/`에 배치.
VS2022가 없으면 상위 저장소 `AGENTS.md` 2절처럼 `CMakeUserPresets.json`으로 제너레이터만 바꾼다.

## 설정

`Data/SKSE/Plugins/ActionMagic.ini`. 없으면 첫 실행 때 기본값으로 생성된다.
MO2에서는 `overwrite` 폴더에 생긴다.

| 섹션 | 키 | 기본값 | 설명 |
|---|---|---|---|
| General | `bEnabled` | `true` | 전체 on/off |
| General | `bShowNotifications` | `true` | 선택한 주문을 좌상단 알림으로 표시 |
| Controls | `iModifierKey` | `42` (왼쪽 Shift) | 순환할 때 누르고 있어야 하는 키. `0`이면 수정키 없음 |
| Controls | `iNextKey` | `265` (휠 아래) | 다음 주문 |
| Controls | `iPrevKey` | `264` (휠 위) | 이전 주문 |
| BareHand | `sTargetHand` | `Right` | 맨손 순환 시 우선 장착할 손 |

키 코드는 SKSE/SkyUI MCM과 같은 번호 체계다 (키보드 0-255, 마우스 256-265, 게임패드 266-281).

## 로그

`Documents/My Games/Skyrim Special Edition/SKSE/ActionMagic.log`

순환할 때마다 `Cycled Right hand -> <주문 이름> (<FormID>)` 줄이 남는다.
