# AIR MicroSoC audio runtime — GPT 5.6 Terra 수정

마이크로 SoC에서도 예측 가능한 오디오 프레임 처리를 목표로 AIR를 다시 정의한 예시다.
실시간 본선은 **DMA + CPU/DSP Q31 fixed-point** 로만 구성하고, GPU는 deadline을
침범하지 않는 선택 분석 작업에만 쓴다.

## 실행 경로

```text
I2S / Bluetooth DMA -> Q31 전처리 -> codec backend -> quantize -> transport adapter
                                            \
                                             -> optional GPU analysis (best effort)
```

- 지원 프레임: 48 kHz, mono/stereo, 7.5 ms 또는 10 ms
- 오디오 워커는 고정 크기 `AudioFrame` 과 SPSC `FrameRing` 을 사용한다. 프레임 처리 중
  동적 할당이나 뮤텍스가 필요 없다.
- `MicroSoCRuntime` 은 각 CPU 단계가 소비할 최악 실행 시간(WCET)을 누적으로 계산한다.
  전체 예산을 넘는 프레임은 처리하지 않는다.
- GPU compute는 `Analysis` 요청만 받을 수 있다. codec, quantize, packetize는 GPU가
  있더라도 CPU/DSP에서 끝낸다.

## 코덱과 전송의 경계

AIR packet은 실험용 사설 포맷이다. 표준 LC3 프레임이나 LE Audio BAP 패킷으로
표시하면 안 된다. 실제 제품에서는 다음처럼 backend를 분리한다.

| 목적 | codec backend | transport adapter |
| --- | --- | --- |
| LE Audio | 규격에 맞는 LC3 encoder | Bluetooth BAP / ISO |
| LDAC 기기 연동 | 플랫폼 또는 라이선스가 허용하는 backend | A2DP / 플랫폼 경로 |
| AIR 실험 | 이 저장소의 AIR test codec | AIR test packet |

무선 종단간 지연은 codec frame 자체가 7.5 ms 또는 10 ms이므로 “sub-1 ms”로
보장하지 않는다. sub-1 ms 목표는 이 runtime 내부 DSP 단계의 처리 시간으로만 다룬다.

## 빌드와 검사

```bash
cmake -S . -B build -DAIR_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

이 코드는 하드웨어 코덱 구현체나 Bluetooth stack을 포함하지 않는다. 플랫폼별 backend는
프레임 deadline 계약을 지키는 별도 모듈로 연결하면 된다.
