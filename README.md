 ![language](https://img.shields.io/badge/language-C++-orange.svg) ![build](https://img.shields.io/badge/build-Windows-blue.svg) ![build](https://img.shields.io/badge/build-linux-FF1010.svg)

# NBLI Image Compression

**NBLI** (new-bee lossless image, **v0.4**) is a fast, better lossless compression algorithm, which support both **RGB 24-bit** and **Gray 8-bit** image. This repo provide:

- **[fNBLI](./fNBLI.exe)** : The optimized fast NBLI
- **[NBLI](./src_NBLI)** : The prototype of NBLI

　

Compared to the popular state-of-the-art lossless image formats, **[fNBLI](./fNBLI.exe)** is outstanding in both speed and compression ratio.

|      Lossless Formats       | Compressed Size (vs. PNG) | Compress Speed | Decompress Speed |
| :-------------------------: | :-----------------------: | :------------: | :--------------: |
|        **BMF** (-s)         |          -35.07%          |    1.3 MB/s    |     1.5 MB/s     |
|         **Gralic**          |          -34.32%          |    4.2 MB/s    |     3.5 MB/s     |
| **[NBLI](./src_NBLI)** (-g) |        **-30.37**%        |   14.7 MB/s    |    17.0 MB/s     |
|     **JPEG-XL** (-e 8)      |          -30.25%          |    0.6 MB/s    |    14.5 MB/s     |
|  **[fNBLI](./fNBLI.exe)**   |        **-29.79**%        | **94.8** MB/s  |  **149.1** MB/s  |
|     **JPEG-XL** (-e 6)      |          -28.63%          |    2.4 MB/s    |    19.1 MB/s     |
|     **JPEG-XL** (-e 3)      |          -26.53%          |   19.2 MB/s    |    27.9 MB/s     |
|        **WEBP** (-5)        |          -22.53%          |    3.7 MB/s    |    82.8 MB/s     |
|     **JPEG-XL** (-e 1)      |          -13.49%          |   95.2 MB/s    |    71.0 MB/s     |
|      **PNG** (optipng)      |           0.00%           |    0.9 MB/s    |    153.7 MB/s    |
|        **JPEG2000**         |          +4.60%           |   12.8 MB/s    |    14.9 MB/s     |
|           **PNG**           |          +5.13%           |   12.2 MB/s    |    121.5 MB/s    |
|        Uncompressed         |         +114.77%          |                |                  |

> The dataset of above comparison is the **[training image set](https://data.vision.ee.ethz.ch/cvl/clic/professional_train_2020.zip)** of [CLIC2021 competition](https://clic.compression.cc/2021/tasks/index.html), which includes **585 RGB 24-bit images**, totaling **4GB** (the downloaded zip file is about 2GB). All above compressor/decompressor are tested in single-threaded, See [Lossless-Image-Compression-Benchmark](https://github.com/WangXuan95/Image-Compression-Benchmark) for detail.

　

# Compile NBLI.exe

### Compile in Windows CMD

If you installed MinGW Compiler for Windows, you can run following command to compile NBLI, getting the executable file [**NBLI.exe**](NBLI.exe)

```powershell
g++ src\NBLI\main.cpp src\imageio\*.c src\imageio\uPNG\*.c src\NBLI\NBLI.cpp -static -O3 -Wall -o NBLI.exe
```

### Compile in Linux

Run following command to compile NBLI, getting the binary file [**NBLI**](./NBLI)

```bash
g++ src/NBLI/main.cpp  src/imageio/*.c src/imageio/uPNG/*.c src/NBLI/NBLI.cpp -static -O3 -Wall -o NBLI
```

　

# Use NBLI.exe

Run [**NBLI.exe**](./NBLI.exe) without any parameters to display its usage:

```bash
> .\NBLI.exe
|----------------------------------------------------------------------------------|
| NBLI : new-bee lossless image codec (single-threaded, v0.4, 202409)              |
|   by https://github.com/WangXuan95/                                              |
|----------------------------------------------------------------------------------|
| Usage:                                                                           |
|   NBLI [-switches]  <in1> [-o <out1>]  [<in2> [-o <out2]]  ...                   |
|                                                                                  |
| To compress:                                                                     |
|   <in>  can be .pgm, .ppm, .pnm, or .png                                         |
|   <out> can be .nbli. It will be generated if not specified.                     |
|                                                                                  |
| To decompress:                                                                   |
|   <in>  can be .nbli                                                             |
|   <out> can be .pgm, .ppm, .pnm, or .png. It will be generated if not specified. |
|                                                                                  |
| switches: -v   : verbose                                                         |
|           -f   : force overwrite of output file                                  |
|           -x   : putting CRC32 when compressing                                  |
|           -0~7 : distortion level. 0=lossless (default), 1~7=lossy               |
|           -g   : use golomb arithmetic coding tree instead of ANS coding (slower)|
|           -a   : use advanced predictor (extremely slow)                         |
|----------------------------------------------------------------------------------|
```

Take the images I provided in the [./image/](./image/) folder as examples :

Compress a .png file to a .nbli file :

```bash
NBLI.exe -vf image\RGB.png -o RGB.nbli
```

Compress a .png file to a .nbli file, use Golomb coding tree (-g) and Advanced predictor (-a) to obtain higher compression ratio :

```bash
NBLI.exe -vfga image\RGB.png -o RGB.nbli
```

Compress a .png file to a .nbli file, set distortion level to 1 (lossy!!) :

```bash
NBLI.exe -vf1 image\RGB.png -o RGB.nbli
```

Decompress the .nbli file back to a .png file :

```bash
NBLI.exe -vf RGB.nbli -o RGB.png
```

　

# Use fNBLI.exe

Run [**fNBLI.exe**](./fNBLI.exe) without any parameters to display its usage:

```bash
> .\fNBLI.exe
|----------------------------------------------------------------------------------|
| fNBLI : fast new-bee lossless image codec (single-threaded, v0.4, 202409)        |
|   by https://github.com/WangXuan95/                                              |
|----------------------------------------------------------------------------------|
| this CPU: AVX2 supported                                                         |
|----------------------------------------------------------------------------------|
| Usage:                                                                           |
|   fNBLI [-switches]  <in1> [-o <out1>]  [<in2> [-o <out2]]  ...                  |
|                                                                                  |
| To compress:                                                                     |
|   <in>  can be .pgm, .ppm, .pnm, or .png                                         |
|   <out> can be .fnbli. It will be generated if not specified.                    |
|                                                                                  |
| To decompress:                                                                   |
|   <in>  can be .fnbli                                                            |
|   <out> can be .pgm, .ppm, .pnm, or .png. It will be generated if not specified. |
|                                                                                  |
| switches: -v : verbose                                                           |
|           -f : force overwrite of output file                                    |
|           -x : putting CRC32 when compressing                                    |
|----------------------------------------------------------------------------------|
```

Take the images I provided in the [./image/](./image/) folder as examples :

Compress a .pgm file (Gray image) to a .fnbli file :

```bash
fNBLI.exe -vf image\Gray.pgm -o Gray.fnbli
```

Compress a .ppm file (RGB image) to a .fnbli file :

```bash
fNBLI.exe -vf image\RGB.ppm -o RGB.fnbli
```

Compress a .png file to .fnbli file :

```bash
fNBLI.exe -vf image\RGB.png -o RGB.fnbli
```

Decompress the .fnbli file back to a .ppm file :

```bash
fNBLI.exe -vf RGB.fnbli -o RGB.ppm
```

Decompress the .fnbli file back to a .png file :

```bash
fNBLI.exe -vf RGB.fnbli -o RGB.png
```

　

# Acknowledgments

Thank the bottle of beer I drank on the evening after my PhD dissertation, which inspired me to come up with a novel parallel entropy encoding method that does not affect the compression ratio, which is the basis of [fNBLI](./fNBLI.exe). 

As for [NBLI](./NBLI.exe), Its main method is not novel. The following works and peoples need to be mentioned:

- [ANS-entropy-coding](https://en.wikipedia.org/wiki/Asymmetric_numeral_systems) by [Jarosław Duda](https://en.wikipedia.org/wiki/Jarosław_Duda_(computer_scientist)), [Yann Collet](https://github.com/Cyan4973), et al.
- CALIC: Context-based, adaptive, lossless image coding: https://ieeexplore.ieee.org/abstract/document/585919 by X. Wu et al.
- EDP: Edge-directed prediction for lossless compression of natural images: https://ieeexplore.ieee.org/abstract/document/923277 by Xin Li et al.
- Glicbawls: Gray Level Image Compression by Adaptive Weighted Least Squares: https://www.researchgate.net/publication/221578041_Glicbawls_-_Grey_Level_Image_Compression_by_Adaptive_Weighted_Least_Squares by Bernd Meyer et al.
- MRP: Lossless coding using variable block-size adaptive prediction optimized for each image: https://ieeexplore.ieee.org/document/7078076 by Ichiro Matsuda et al.
- A lossless color image compression method based on a new reversible color transform: https://ieeexplore.ieee.org/document/6410808 by Seyun Kim et al.
- [JPEG-LS extension](https://github.com/WangXuan95/JPEG-LS_extension) ([ITU-T T.870](https://www.itu.int/rec/T-REC-T.870/en))
- The experts of data compression, [Alex Rhatushnyak](http://qlic.altervista.org/), [Matt Mahoney](https://mattmahoney.net/), [Igor Pavlov](https://7-zip.org/), et al.
- And of course, Claude E. Shannon.

