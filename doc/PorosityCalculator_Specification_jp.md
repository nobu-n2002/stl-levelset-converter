# Porosity Calculator 仕様書

## 概要

ポロシティ計算機は、STLファイルで表される幾何学的構造のポロシティを、Signed Distance Fields（SDF）を使用して計算するためのソフトウェアツールです。このドキュメントでは、ポロシティ計算機の機能、入力要件、出力形式、およびその他の関連する詳細について説明します。

## ライセンス

ポロシティ計算機はMITライセンスの下で公開されています。ライセンスの詳細は次のとおりです：

```txt
MIT License

著作権（c）2024 Nobuto NAKAMICHI

次の条件に従って、このソフトウェアと関連文書ファイル（以下「ソフトウェア」といいます。）のコピーを入手するすべての人に対して、
無償で許可されます。ソフトウェアを制限なしに扱うこと、
これには、制限なしに、使用、コピー、変更、マージ、公開、配布、
サブライセンス、および/または販売の権利が含まれます。
...
```

## 設定ファイル（config.txt）

ポロシティ計算機には、入力パラメータを指定するための「config.txt」という名前の設定ファイルが必要です。設定ファイルの形式は次のとおりです：

- `stlFilePath`: 幾何構造を含むSTLファイルへのパス
- `outputCsvFileName`: 出力データが書き込まれるCSVファイルへのパス
- `outputVtkFilePath`: 出力データが書き込まれるVTKファイルへのパス（オプション）
- `boundsFactor`: バウンディングボックスサイズを調整するためのスケーリングファクターを表す6つの浮動小数点数値のリスト
- `grid`: グリッド解像度を指定する整数値
- `axis`: グリッド生成のための軸を指定する整数値
- `thickness`: 正規化のための厚さパラメータを表す浮動小数点数値
- `outputVtk`: VTK形式で出力データを出力するかどうかを示すブール値（true/false）
- `numThreads`: 並列計算に使用するスレッド数を指定する整数値

設定ファイルの例：

```txt
stlFilePath = example.stl
outputCsvFileName = output.csv
outputVtkFilePath = output.vtk
boundsFactor = 1.2 1.2 1.2 1.2 1.2 1.2
grid = 100
axis = 0
thickness = 1.5
outputVtk = true
numThreads = 4
```

## 入力要件

ポロシティ計算機には、以下の入力が必要です：

- 幾何構造のSTLファイル
- 入力パラメータを指定する設定ファイル

## 出力形式

ポロシティ計算機は、以下の出力を生成します：

- セルの寸法とスカラー値を含むCSVファイル
- （オプション）処理されたデータを含むVTKファイル

## 機能

ポロシティ計算機は、以下の手順を実行します：

1. 設定ファイルから入力パラメータを読み込みます。
2. STLファイルを読み込んで幾何構造を処理します。
3. Signed Distance Fields（SDF）を使用してポロシティを計算します。
4. CSVおよび/またはVTK形式の出力データを生成します。
5. 処理されたデータに関する情報を提供します。

## コード仕様

## 使用されるライブラリ

- VTK（Visualization Toolkit）
- OpenMP

### 関数とクラス

1. `bool readConfigFile(const std::string& configFilePath, std::string& stlFilePath, std::string& outputCsvFileName, std::string& outputVtkFilePath, std::vector<double>& boundsFactor, int& grid, int& axis, double& thickness, bool& outputVtk, int& numThreads)`
   - 説明：指定された設定ファイルから必要なパラメータを読み込みます。
   - 引数：
     - `configFilePath`：設定ファイルへのパス
     - `stlFilePath`：STLファイルへのパス
     - `outputCsvFileName`：CSVファイルの出力パス
     - `outputVtkFilePath`：VTKファイルの出力パス
     - `boundsFactor`：バウンディングボックスのスケーリングファクターのリスト
     - `grid`：グリッドセルの数
     - `axis`：軸のインデックス
     - `thickness`：厚さの値
     - `outputVtk`：VTKファイルの出力フラグ
     - `numThreads`：スレッド数
   - 戻り値：設定ファイルの読み込みが成功したかどうかを示すブール値。

2. `vtkSmartPointer<vtkImageData> processSTLFile(vtkSmartPointer<vtkPolyData> polyData, const std::vector<double>& boundsFactor, int grid, int axis)`
   - 説明：STLファイルから読み込んだポリゴンデータを処理し、VTK形式のイメージデータを生成します。
   - 引数：
     - `polyData`：STLファイルから読み込んだポリゴンデータ
     - `boundsFactor`：バウンディングボックスのスケーリングファクターのリスト
     - `grid`：グリッドセルの数
     - `axis`：軸のインデックス
   - 戻り値：処理されたVTK形式のイメージデータ。

3. `vtkSmartPointer<vtkDoubleArray> calculate_porosity(vtkSmartPointer<vtkPolyData> polyData, vtkSmartPointer<vtkImageData> imageData, double thickness)`
   - 説明：Signed Distance Fields（SDF）を使用してポリゴンデータのポロシティを計算します。
   - 引数：
     - `polyData`：入力ポリゴンデータ
     - `imageData`：入力イメージデータ
     - `thickness`：厚さパラメータ
   - 戻り値：計算されたポロシティデータ。

4. `void outputDataDetails(vtkImageData* imageData)`
   - 説明：出力データの詳細情報を出力します。
   - 引数：
     - `imageData`：出力イメージデータ

5. `void writeImageDataToCSV(vtkImageData* imageData, const std::string& outputCsvFileName)`
   - 説明：イメージデータをCSVファイルに書き込みます。
   - 引数：
     - `imageData`：出力イメージデータ
     - `outputCsvFileName`：CSVファイルの出力パス

6. `void writeImageDataToFile(vtkImageData* imageData, const std::string& outputVtkFilePath)`
   - 説明：イメージデータをVTKファイルに書き込みます。
   - 引数：
     - `imageData`：出力イメージデータ
     - `outputVtkFilePath`：VTKファイルの出力パス

7. `int main()`
   - 説明：メイン関数。設定ファイルを読み込み、STLファイルからポリゴンデータを処理し、ポロシティを計算し、ファイルに出力します。

---

さらなる説明や追加の詳細が必要な場合はお知らせください。
