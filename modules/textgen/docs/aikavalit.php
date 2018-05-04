<?php

require("config.php");

$width = 800;
$height = 80;
$rows = 4;

$img = imagecreate($width+1,$height+1);
$white = imagecolorallocate($img,255,255,255);
$black = imagecolorallocate($img,0,0,0);

$dx = $width/24;
$dy = $height/$rows;

# Matriisi

imageline($img,0,0,$width,0,$black);
imageline($img,0,$dy,$width,$dy,$black);
imageline($img,0,2*$dy,$width,2*$dy,$black);
imageline($img,0,3*$dy,$width,3*$dy,$black);
imageline($img,0,4*$dy,$width/2,4*$dy,$black);
imageline($img,$width*3/4,4*$dy,$width,4*$dy,$black);

imageline($img,0,0,0,4*$dy,$black);			// 06
imageline($img,3*$dx,$dy,3*$dx,4*$dy,$black);		// 09
imageline($img,5*$dx,3*$dy,5*$dx,4*$dy,$black);		// 11
imageline($img,6*$dx,2*$dy,6*$dx,3*$dy,$black);		// 12
imageline($img,7*$dx,3*$dy,7*$dx,4*$dy,$black);		// 13
imageline($img,12*$dx,0,12*$dx,4*$dy,$black);		// 18
imageline($img,16*$dx,$dy,16*$dx,3*$dy,$black);		// 22
imageline($img,18*$dx,2*$dy,18*$dx,4*$dy,$black);	// 00
imageline($img,21*$dx,2*$dy,21*$dx,3*$dy,$black);	// 03
imageline($img,24*$dx,0,24*$dx,4*$dy,$black);		// 06

# Tekstit

$font = "$TTF_FONT_DIR/ArialNarrow.ttf";
$size = 10;

ImageCenteredText($img,$size,6*$dx,0.5*$dy,$black,$font,"Päivä");
ImageCenteredText($img,$size,18*$dx,0.5*$dy,$black,$font,"Yö");

ImageCenteredText($img,$size,1.5*$dx,1.5*$dy,$black,$font,"Aamu");
ImageCenteredText($img,$size,7.5*$dx,1.5*$dy,$black,$font,"Päivä");
ImageCenteredText($img,$size,14*$dx,1.5*$dy,$black,$font,"Ilta");
ImageCenteredText($img,$size,20*$dx,1.5*$dy,$black,$font,"Yö");

ImageCenteredText($img,$size,1.5*$dx,2.5*$dy,$black,$font,"Aamu");
ImageCenteredText($img,$size,4.5*$dx,2.5*$dy,$black,$font,"Aamupäivä");
ImageCenteredText($img,$size,9*$dx,2.5*$dy,$black,$font,"Iltapäivä");
ImageCenteredText($img,$size,14*$dx,2.5*$dy,$black,$font,"Ilta");
ImageCenteredText($img,$size,17*$dx,2.5*$dy,$black,$font,"Iltayö");
ImageCenteredText($img,$size,19.5*$dx,2.5*$dy,$black,$font,"Keskiyö");
ImageCenteredText($img,$size,22.5*$dx,2.5*$dy,$black,$font,"Aamuyö");

ImageCenteredText($img,$size,1.5*$dx,3.5*$dy,$black,$font,"Aamu");
ImageCenteredText($img,$size,4*$dx,3.5*$dy,$black,$font,"Aamupäivä");
ImageCenteredText($img,$size,6*$dx,3.5*$dy,$black,$font,"Keskipäivä");
ImageCenteredText($img,$size,9.5*$dx,3.5*$dy,$black,$font,"Iltapäivä");
ImageCenteredText($img,$size,21*$dx,3.5*$dy,$black,$font,"Yö");

$yoffset = 20;
$xoffset = 20;
$out = imagecreate($width+1+2*$xoffset,$height+$yoffset+1);
$white = imagecolorallocate($out,255,255,255);
$black = imagecolorallocate($out,0,0,0);

imagecopy($out,$img,$xoffset,$yoffset,0,0,$width+1,$height+1);

ImageCenteredText($out,$size,$xoffset+0*$dx,$yoffset-$size,$black,$font,"06");
ImageCenteredText($out,$size,$xoffset+1*$dx,$yoffset-$size,$black,$font,"07");
ImageCenteredText($out,$size,$xoffset+2*$dx,$yoffset-$size,$black,$font,"08");
ImageCenteredText($out,$size,$xoffset+3*$dx,$yoffset-$size,$black,$font,"09");
ImageCenteredText($out,$size,$xoffset+4*$dx,$yoffset-$size,$black,$font,"10");
ImageCenteredText($out,$size,$xoffset+5*$dx,$yoffset-$size,$black,$font,"11");
ImageCenteredText($out,$size,$xoffset+6*$dx,$yoffset-$size,$black,$font,"12");
ImageCenteredText($out,$size,$xoffset+7*$dx,$yoffset-$size,$black,$font,"13");
ImageCenteredText($out,$size,$xoffset+8*$dx,$yoffset-$size,$black,$font,"14");
ImageCenteredText($out,$size,$xoffset+9*$dx,$yoffset-$size,$black,$font,"15");
ImageCenteredText($out,$size,$xoffset+10*$dx,$yoffset-$size,$black,$font,"16");
ImageCenteredText($out,$size,$xoffset+11*$dx,$yoffset-$size,$black,$font,"17");
ImageCenteredText($out,$size,$xoffset+12*$dx,$yoffset-$size,$black,$font,"18");
ImageCenteredText($out,$size,$xoffset+13*$dx,$yoffset-$size,$black,$font,"19");
ImageCenteredText($out,$size,$xoffset+14*$dx,$yoffset-$size,$black,$font,"20");
ImageCenteredText($out,$size,$xoffset+15*$dx,$yoffset-$size,$black,$font,"21");
ImageCenteredText($out,$size,$xoffset+16*$dx,$yoffset-$size,$black,$font,"22");
ImageCenteredText($out,$size,$xoffset+17*$dx,$yoffset-$size,$black,$font,"23");
ImageCenteredText($out,$size,$xoffset+18*$dx,$yoffset-$size,$black,$font,"00");
ImageCenteredText($out,$size,$xoffset+19*$dx,$yoffset-$size,$black,$font,"01");
ImageCenteredText($out,$size,$xoffset+20*$dx,$yoffset-$size,$black,$font,"02");
ImageCenteredText($out,$size,$xoffset+21*$dx,$yoffset-$size,$black,$font,"03");
ImageCenteredText($out,$size,$xoffset+22*$dx,$yoffset-$size,$black,$font,"04");
ImageCenteredText($out,$size,$xoffset+23*$dx,$yoffset-$size,$black,$font,"05");
ImageCenteredText($out,$size,$xoffset+24*$dx,$yoffset-$size,$black,$font,"06");

header("Content-type: image/png");
imagepng($out);


// ----------------------------------------------------------------------
/*!
 * \brief An utility function to draw centered text
 *
 * \param img The image to draw into
 * \param size The size of the font
 * \param x The X-coordinate of the center
 * \param y The Y-coordinate of the center
 * \param color The GD color number for the text
 * \param font The font file
 * \param text The text itself
 */
// ----------------------------------------------------------------------

function ImageCenteredText(&$img,$size,$x,$y,$color,$font,$text)
{
  $bbox = ImageTTFBBox($size,0,$font,$text);
  $cx = ($bbox[0]+$bbox[2])/2;
  $cy = ($bbox[1]+$bbox[7])/2;

  # The minus logic is strange, I know, but it seems to work
  ImageTTFText($img,$size,0,floor($x-$cx-0.5),floor($y-$cy-0.5),$color,$font,$text);
}



?>