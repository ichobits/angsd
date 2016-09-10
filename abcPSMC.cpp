/*
  This is a class that dumps psmc file output

 */
#include <assert.h>

#include "analysisFunction.h"
#include "shared.h"
#include <htslib/kstring.h>
#include "abcPSMC.h"



void abcPSMC::printArg(FILE *argFile){
  fprintf(argFile,"------------------------\n%s:\n",__FILE__);
  fprintf(argFile,"\t-writePSMC\t%d\n",dopsmc);
  fprintf(argFile,"\t1:  (still beta, not really working)\n");
}

void abcPSMC::algoJoint(double **liks,int nsites,int *keepSites,psmcRes *r,int noTrans) {
  //  fprintf(stderr,"[%s]\n",__FUNCTION__);
  int myCounter =0;
  assert(liks);
  for(int it=0; it<nsites; it++) {//loop over sites
    if(keepSites[it]==0)
      continue;
    // AA,AC,AG,AT,CC,CG,CT,GG,GT,TT
    //  0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    //  =. -. *. -. =. -. *. =. -. =
    const char homoz[] = {0,4,7,9};//=
    const char transverz[] = {1,3,5,8};//-
    const char transitz[] ={2,6};//* 
    double homo =log(0);
    double het= log(0);

    for(int i=0;i<4;i++){
      homo +=angsd::addProtect2(homo,liks[it][homoz[i]]);
      het += angsd::addProtect2(het,liks[it][transverz[i]]);
    }
    for(int i=0;(noTrans==0&&i<2);i++)
      het += angsd::addProtect2(het,liks[it][transitz[i]]);

    r->oklist[it] = 1;
    r->pLikes[it*2] = homo;
    r->pLikes[it*2+1] = het;
  }

}



void abcPSMC::run(funkyPars *pars){
  if(dopsmc==0)
    return ;
  psmcRes *r = new psmcRes;
  r->oklist = (char*) calloc(pars->numSites,sizeof(char));
  r->pLikes = (double *) calloc(pars->numSites,2*sizeof(double));
  algoJoint(pars->likes,pars->numSites,pars->keepSites,r,noTrans);
  pars->extras[index] = r;
}

void abcPSMC::clean(funkyPars *pars){
  if(dopsmc==0)
    return;
    

}

void abcPSMC::print(funkyPars *pars){
  if(dopsmc==0)
    return;

}

void abcPSMC::getOptions(argStruct *arguments){

  dopsmc=angsd::getArg("-doPSMC",dopsmc,arguments);
  if(dopsmc==0)
    return;
  int gl =0;
  gl=angsd::getArg("-gl",gl,arguments);

  if(gl==0||INPUT_BEAGLE||INPUT_VCF_GP){
    fprintf(stderr,"\nPotential problem. We need genotypes for dumping psmc\n\n");
    exit(0);
  }
  noTrans = angsd::getArg("-noTrans",noTrans,arguments);
  
}

abcPSMC::abcPSMC(const char *outfiles,argStruct *arguments,int inputtype){
    noTrans = 0;
    tmpChr = NULL;
    dopsmc =0;
    outfileSAF = NULL;
    outfileSAFPOS = NULL;
    outfileSAFIDX = NULL;
    nnnSites = 0;
    if(arguments->argc==2){
    if(!strcasecmp(arguments->argv[1],"-dopsmc")){
    printArg(stdout);
    exit(0);
  }else
      return;
  }


  getOptions(arguments);

  if(dopsmc==0){
    shouldRun[index] =0;
    return;
  }
  printArg(arguments->argumentFile);

  const char *SAF = ".saf.gz";
  const char *SAFPOS =".saf.pos.gz";
  const char *SAFIDX =".saf.idx";

  outfileSAF =  aio::openFileBG(outfiles,SAF);
  outfileSAFPOS =  aio::openFileBG(outfiles,SAFPOS);
  outfileSAFIDX = aio::openFile(outfiles,SAFIDX);
  char buf[8]="psmcv3";
  aio::bgzf_write(outfileSAF,buf,8);
  aio::bgzf_write(outfileSAFPOS,buf,8);
  fwrite(buf,1,8,outfileSAFIDX);
  offs[0] = bgzf_tell(outfileSAFPOS);
  offs[1] = bgzf_tell(outfileSAF);
  
}

void abcPSMC::writeAll(){
  assert(outfileSAF!=NULL);
  assert(outfileSAFIDX!=NULL);
  assert(outfileSAFPOS!=NULL);
  //  fprintf(stderr,"nnnSites:%d\n",nnnSites);
  if(nnnSites!=0&&tmpChr!=NULL){
    size_t clen = strlen(tmpChr);
    fwrite(&clen,sizeof(size_t),1,outfileSAFIDX);
    fwrite(tmpChr,1,clen,outfileSAFIDX);
    size_t tt = nnnSites;
    fwrite(&tt,sizeof(size_t),1,outfileSAFIDX);
    fwrite(offs,sizeof(int64_t),2,outfileSAFIDX);
  }//else
   // fprintf(stderr,"enpty chr\n");
  //reset
  offs[0] = bgzf_tell(outfileSAFPOS);
  offs[1] = bgzf_tell(outfileSAF);
  nnnSites=0;
}


abcPSMC::~abcPSMC(){
  if(dopsmc)
    writeAll();
}


void abcPSMC::changeChr(int refId) {
  if(dopsmc){
    writeAll();
    free(tmpChr);
    tmpChr = strdup(header->target_name[refId]);
  }
}
