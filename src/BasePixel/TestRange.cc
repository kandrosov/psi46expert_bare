/*!
 * \file TestRange.cc
 * \brief Implementation of TestRange class.
 *
 * \b Changelog
 * 22-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Now using definitions from PsiCommon.h.
 * 15-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Inheritence from TObject removed due to compability issues.
 */




#include <fstream>

#include "psi/log.h"
#include "TestRange.h"

TestRange::TestRange()
{
    for (int iRoc = 0; iRoc < psi::MODULENUMROCS; iRoc++)
	{
        for (int iCol = 0; iCol < psi::ROCNUMCOLS; iCol++)
		{
            for (int iRow = 0; iRow < psi::ROCNUMROWS; iRow++) pixel[iRoc][iCol][iRow] = false;
		}
	}
}


void TestRange::CompleteRange()
{
    for (int iRoc = 0; iRoc < psi::MODULENUMROCS; iRoc++)
	{
		CompleteRoc(iRoc);
	}
}


void TestRange::CompleteRoc(int iRoc)
{
    for (int iCol = 0; iCol < psi::ROCNUMCOLS; iCol++)
	{
        for (int iRow = 0; iRow < psi::ROCNUMROWS; iRow++) pixel[iRoc][iCol][iRow] = true;
	}
}


void TestRange::AddPixel(int iRoc, int col, int row)
{
	pixel[iRoc][col][row] = true;
}


void TestRange::RemovePixel(int iRoc, int col, int row)
{
	pixel[iRoc][col][row] = false;
}


bool TestRange::IncludesPixel(int iRoc, int col, int row)
{
	return pixel[iRoc][col][row];
}


bool TestRange::IncludesRoc(int iRoc)
{
	bool result = false;
    for (int k = 0; k < psi::ROCNUMCOLS; k++)
	{
        for (int l = 0; l < psi::ROCNUMROWS; l++)
		{
			if (pixel[iRoc][k][l]) result = true;
		}
	}
	return result;
}


bool TestRange::IncludesDoubleColumn(int iRoc, int doubleColumn)
{
	bool result = false;
	for (int k = doubleColumn*2; k < doubleColumn*2+2; k++)
	{
        for (int l = 0; l < psi::ROCNUMROWS; l++)
		{
			if (pixel[iRoc][k][l]) result = true;
		}
	}
	return result;
}


bool TestRange::IncludesColumn(int column)
{
	bool result = false;
    for (int iRoc = 0; iRoc < psi::MODULENUMROCS; iRoc++)
	{
        for (int l = 0; l < psi::ROCNUMROWS; l++)
		{
			if (pixel[iRoc][column][l]) result = true;
		}
	}
	return result;
}


bool TestRange::IncludesColumn(int iRoc, int column)
{
	bool result = false;
    for (int l = 0; l < psi::ROCNUMROWS; l++)
	{
		if (pixel[iRoc][column][l]) result = true;
	}
	return result;
}

bool TestRange::ExcludesColumn(int iRoc, int column)
{
	bool result = false;
    for (int l = 0; l < psi::ROCNUMROWS; l++)
	{
		pixel[iRoc][column][l] = false;
	}
	return result;
}

bool TestRange::ExcludesRow(int iRoc, int row)
{
	bool result = false;
    for (int l = 0; l < psi::ROCNUMCOLS; l++)
	{
		pixel[iRoc][l][row] = false;
	}
	return result;
}

bool TestRange::ExcludesRoc(int iRoc){
	
	bool result = false;
    for (int l = 0; l < psi::ROCNUMROWS; l++) {
      for (int m = 0; m < psi::ROCNUMCOLS; m++){
		pixel[iRoc][m][l] = false;
	  }
	}  
	return result;	
}

void TestRange::ApplyMaskFile(const char *fileName){
    std::string fname(fileName);
  
  int roc, col, row;
  std::string keyWord;
  char line[1000];

  std::ifstream maskFile;
  maskFile.open(fname.c_str());
  
  if (maskFile.bad()) 
  {
      psi::LogInfo() << "!!!!!!!!!  ----> Could not open file "<<fname<<" to read pixel mask\n";
      return;
  }  
  
  psi::LogInfo() << "Reading pixel mask from "<< fname << std::endl;
  
  while(maskFile.good()){
    maskFile>>keyWord;
    if (keyWord == "#"){
       maskFile.getline(line,60, '\n');
       psi::LogInfo() << "# "<<line << std::endl;// ignore rows starting with "#" = comment
    }
    else if(keyWord == "pix"){
       maskFile>>roc>>col>>row;
       psi::LogInfo() << "Exclude "<<keyWord<<" "<<roc<<" "<<col<<" "<<row<<std::endl;
       if ((roc >= 0)&&(roc < psi::MODULENUMROCS)&&(col >= 0)&&(col < psi::ROCNUMCOLS)&&(row >= 0)&&(row < psi::ROCNUMROWS)){
         RemovePixel(roc,col,row);
       }else{
         psi::LogInfo() << "!!!!!!!!!  ----> Pixel number out of range: "<<keyWord<<" "<<roc<<" "<<col<<" "<<row<<std::endl;
       }
    }else if(keyWord == "col"){
       maskFile>>roc>>col;
       psi::LogInfo() << "Exclude "<<keyWord<<" "<<roc<<" "<<col<<std::endl;
       if ((roc >= 0)&&(roc < psi::MODULENUMROCS)&&(col >= 0)&&(col < psi::ROCNUMCOLS)){
         ExcludesColumn(roc,col);      
       }else{
         psi::LogInfo() << "!!!!!!!!!  ----> Pixel number out of range: "<<keyWord<<" "<<roc<<" "<<col<<std::endl;
       }
    }else if(keyWord == "row"){
       maskFile>>roc>>row;
       psi::LogInfo() << "Exclude "<<keyWord<<" "<<roc<<" "<<row<<std::endl;
       if ((roc >= 0)&&(roc < psi::MODULENUMROCS)&&(row >= 0)&&(row < psi::ROCNUMROWS)){
         ExcludesRow(roc,row);   
       }else{
         psi::LogInfo() << "!!!!!!!!!  ----> Pixel number out of range: "<<keyWord<<" "<<roc<<" "<<row<<std::endl;
       }
    }else if(keyWord == "roc"){
       maskFile>>roc;
       psi::LogInfo() << "Exclude "<<keyWord<<" "<<roc<<std::endl;
       if ((roc >= 0)&&(roc < psi::MODULENUMROCS)){
         ExcludesRoc(roc);         
       }else{
         psi::LogInfo() << "!!!!!!!!!  ----> Pixel number out of range: "<<keyWord<<" "<<roc<<" "<<col<<" "<<row<<std::endl;
       }       
    }
    keyWord = "";
  }
  
  maskFile.close();
  
  return;
  
}

void TestRange::Print()
{
    for (int i = 0; i < psi::MODULENUMROCS; i++)
	{
        for (int k = 0; k < psi::ROCNUMCOLS; k++)
		{
            for (int l = 0; l < psi::ROCNUMROWS; l++)
            {
                if (pixel[i][k][l])
                    psi::LogInfo() << "pixel " << i << " " << k << " " << l << std::endl;
			}
		}
	}
}
