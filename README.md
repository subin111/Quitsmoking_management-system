# Quitsmoking_management system


   ## 목차
   
   1. 개요
   2. 프로젝트 구성
   3. 구현 과정
   4. 구현 결과
   5. 결론

## 1. 개요

* 추진 배경
   - 흡연의 중독성이 매우 강한 만큼, 금연 의지를 가지고 있는 사람들의 금연을 돕고자 프로젝트를 기획하게 되었음.
   - 웨어러블 형태의 스마트 디바이스로 착용 시간동안 사용자에게 24시 밀착 도움을 제공함.

* 개발 목표
  - 단순 금연홍보용품으로 존재하는 금연 팔찌에 스마트 기능을 탑재한 웨어러블 스마트 디바이스
  - 지역 보건소의 금연 클리닉과 연계한 스마트 디바이스 + 금연관리 시스템 구축

    => 체계적인 금연관리 시스템을 통해 디바이스 사용자 간의 경쟁의식 자극하고,   금연 성공 보상 제공을 통한 성취감을 느끼게 하는 등 금연 성공률을 높일 수 있는 여러 방법들을 제공하고자 함.

## 2. 프로젝트 구성

* 시스템 구성도

  <img src="https://github.com/subin111/Quitsmoking_management-system/assets/143717650/f56d9c26-b3dc-4a90-98c1-231922db03de" width="550" height="400"/>


## 3. 구현 과정

### 1. 아두이노 (Devive)

   1. 가스 센서
   2. DC 모터
   3. OLED

      가스 센서가 일정 값 이상 감지되면 OLED에 Cigarette detection!! 출력과 동시에 DC 모터 작동 (진동 모터 대신)하며, 손목에 착용하는 웨어러블 디바이스로 시계 기능 추가하였음.
   
   <img src ="https://github.com/subin111/Quitsmoking_management-system/assets/143717650/96c8fd46-b3c1-4ea0-854e-87afb4713cb3" width = "200" height = "250"/>


### 2. Raspberry Pi (Main Server, Database) * 추후 코드 수정

 1. MariaDB & PhpMyAdmin

    디바이스로부터 받은 데이터를 그룹 별 테이블 안에 user1, user2 ... 사용자 별로 하루 동안의 흡연 횟수를 날짜와 함께 저장

    <img src = "https://github.com/subin111/Quitsmoking_management-system/assets/143717650/058b5e95-4d3b-4397-9ce7-efa6790671fa" width = "300" height = "200"/>

    
    <img src ="https://github.com/subin111/Quitsmoking_management-system/assets/143717650/d0798a6a-1b7c-4641-9b65-9ceb93d1495a" width = "300" height = "200"/>


### 3. STM32F429 (Control Center)

   1. LCD

      서버에서 하루 동안의 흡연횟수 정보를 받아 그룹 구성원의 하루 흡연 성공여부를 LCD에 출력 


      "img src = "https://github.com/subin111/Quitsmoking_management-system/assets/143717650/c19bcb52-c280-42fc-9006-54aca1f57441" width = "200" height = "230"/>

      

## 4. 구현 결과

* 시연 영상

  
## 5. 결론

   * 문제점 및 개선점

     1. 한 개의 그룹, 그룹 당 2명으로 인원 제한 => 여러 그룹, 그룹 당 인원 수 제한을 없애 접속 디바이스를 늘려 시스템 확장
        
     2. 전체 사용자들의 카운트 수를 하루 단위로 비교할 수 있는 랭킹 시스템 도입 => 사용자의 경쟁의식을 자극하여 목표를 달성할 수 있도록 함
        
     3. 센터에서 디바이스로의 데이터 전송을 추가 => 금연 센터에서 사용자들을 적극적으로 관리할 수 있도록 함
        ex) LSB님 흡연 횟수를 줄이셔야 합니다.
        ex) 목표 달성 마감일까지 9일 남았습니다.
