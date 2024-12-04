# HYMS
5)이종 단일 디지털 트윈 간 시간과 공간의 연속성 제공 시뮬레이션 기술 개발

### Recommended system
* Windows 10 이상, Visual Studio 2019 (V142)

### install 
* Python 3.10.8 이상
* docker-compose up -d

### Get started 
    ### CreateAtomdll_Samples 
    * 저작 도구에서 사용하는 원자 모델 개발 프로젝트.
    
    ### atomic_model_manager 
    * 원자 모델 등록
      1. 만들어진 원자모델 파일(.Dll, .py)을 projects 안에 프로젝트 명 폴더로 이동
      2. atomic_model_manager.exe 실행 -> update 커멘드 입력
      3. 원자모델의 정상 실행으로 등록 완료 확인
    * 원자 모델 실행기
      
    ### fDTSimServer 
    * 하이브리드 시뮬레이션 엔진 서버

### Use HYMS 
  * 모델 조합기 사용을 위해 도커 컨테이너 실행
  * 시뮬레이션을 위해 서버에 다음 프로그램 실행
      fDTSimServer.exe
      atomic_model_manager.exe
  * 모델 조합기를 매뉴얼에 따라 이용
