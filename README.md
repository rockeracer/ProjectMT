# ProjectMT
개인 제작 게임 엔진

## 구현 내용
* 멀티플랫폼 지원
  - Windows
  - Mac

* VisualStudio 솔루션 파일 자동 생성
  - ./Source/GenerateProjectFiels.py 스크립트를 실행하면 솔루션 파일 생성.
  - 각 모듈의 정보는 모듈별로 Build.json 파일에서 설정.

* Property 시스템
  - 헤더 파일을 파싱해서 Class, Property에 대한 메타데이터 생성.
  - ./Source/HeaderParser

* Rendering 시스템
  - 여러 API 지원
    - DX12 : ./Source/DX12RDI
    - Vulkan : ./Source/VulkanRDI
    - Metal : ./Source/MetalRDI
  - 실제로 사용되는 API와 무관하게 동일한 interface를 사용할 수 있는 구조로 구현.
  - Shader
    - 언리얼 엔진과 같은 매테리얼 그래프를 통해 쉐이더 코드 생성.
    - ./Source/Shader
    - ./Shader
  - Rendering Thread 구현
    - ./Source/RenderCore

* GUI
  - imgui 라이브러리 사용.
  - ./Source/GUI

* Unit Test
  - googletest를 사용하여 유닛테스트 수행.
  - ./Source/Test
