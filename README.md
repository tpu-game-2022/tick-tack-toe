# 課題：強くして
本リポジトリのAIに対して、何らかの工夫をしてください！

* 計算を効率化する
* 新たなアルゴリズムを実装する

5目並べにすると、わかりやすいかもしれません。


![結果画像](image.png)

# 取り組み方
* 本プロジェクトをforkして、取り組んでください。
* GitHub Actions (Actionsのタブ)を機能させて、README.mdに記述された下記のバッチの「tpu-game-2022」を自分のアカウントに差し替えてください。
* readme.md に実施した工夫を記載してください
* 可能であれば、速度等を計測して、具体的な効率化度合い、強さを示してください。
* 納得できるところまでできたところでプルリクを出してください。

[![MSBuild](https://github.com/shuhei-M/tick-tack-toe/actions/workflows/msbuild.yml/badge.svg)](https://github.com/shuhei-M/tick-tack-toe/actions/workflows/msbuild.yml)

（↑のソースコードの「tpu-game-2021」を自分のアカウント名に差し替えてください（２か所））

# 〆切
*/*(日)


# 追加・変更・工夫した点
* min-max戦略, nega-max戦略, alpha-beta法, Nega-Scout法, モンテカルロ法のAIをそれぞれ実装しました。
* モンテカルロ木探索に関しては、スライドを参考にコーディングしたものの、うまく実装できませんでした。  
  モンテカルロ法より弱くなってしまったため、提出したコードのAIにはモンテカルロ法をセットしてあります。
* MassクラスとBordクラスをBord.hに、AIクラス群をAI.hとAI.cppに、GameクラスをGame.hに分けて実装しました。
* AIの性能の差をに比べるため、AIを差し替えやすい様にしました。  
  int型の整数をstaticキャストしてAI::type型の変数ai_typeにセットしています。数字を変えるだけでAIを差し替えられようになっています。
