/***************************************************************************
 *   Copyright (C) 2007 by Yan Zhao   *
 *   yzhao@rana   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <time.h>
#include <limits>


#include "PosMaxEn.h"
#include "CRF.h"
#include <getopt.h>
#include <iomanip>
#include <fstream>


using namespace std;

#include "JTriggerMgr.h";
#include "JEvaluate.h"
#include "JBaseProc.h"
#include "fadd.h"
#include "Trellis.h"


static int verbose_flag;
int trn_flag = 0;
int tst_flag = 0;
clock_t startClk;
extern double g_filter_value;

void ParseCmd(int argc, char* argv[])
{
	cout<<"**************************************************************"<<endl;
	//above is to make output more readable :)
	int c;
	char *stop_char;
	string templateFileName = "template.txt";
	string trainFileName = "train";
	string testFileName = "test";
       while (1)
         {
           static struct option long_options[] =
             {
               /* These options set a flag. */	
		{"bigram",no_argument,  & g_Bigram, 1},
		{"me",no_argument,  & g_me, 1},
		{"crf",no_argument,  & g_crf, 1},
		{"syn", no_argument,  & g_syn_flag, 1},
               {"verbose", no_argument,       &verbose_flag, 1},
               {"brief",   no_argument,       &verbose_flag, 0},
		{"trn", no_argument,       &trn_flag, 1},
               {"tst",   no_argument,       &tst_flag, 1},
		{"gis",  no_argument,       &g_gis, 1},
		{"options", no_argument,  &g_flag_pos_options, 1},

               /* These options don't set a flag.
                  We distinguish them by their indices. */
		{"decoding method",     required_argument,       0, 'd'},
               {"cutoff_count",     required_argument,       0, 'c'},
		{"application", required_argument,       0, 'l'},
		 {"filter_value",  required_argument,       0, 'v'},
               {"gussian_smoothing",  required_argument,       0, 'g'},
	       {"training_file_name",  required_argument,       0, 'r'},
	       {"testing_file_name",  required_argument,       0, 'e'},
               {"template_file",    required_argument, 0, 'f'},
		{"iteration_time",    required_argument, 0, 'i'},
		{"mwu_or_tag", required_argument, 0, 'm'},
		{"output_debug", required_argument, 0, 'o'},
		{"help", no_argument, 0, '?'},
               {0, 0, 0, 0}
             };
           /* getopt_long stores the option index here. */
           int option_index = 0;
     
           c = getopt_long (argc, argv, "g:c:f:l:r:e:i:d:m:v:o:",
                            long_options, &option_index);
     
           /* Detect the end of the options. */
           if (c == -1)
             break;
     
           switch (c)
             {
             case 0:
               /* If this option set a flag, do nothing else now. */
               if (long_options[option_index].flag != 0)
                 break;
               printf ("option %s", long_options[option_index].name);
               if (optarg)
                 printf (" with arg %s", optarg);
               printf ("\n");
               break;
		
		case 'd':
           //    printf ("cutoff_count has been set to  `%s'\n", optarg);
		g_decodingMethod = strtol(optarg,&stop_char,0);
               break;

		case 'o':
           //    printf ("cutoff_count has been set to  `%s'\n", optarg);
		g_debug = strtol(optarg,&stop_char,0);
               break;
		
      
		case 'i':
           //    printf ("cutoff_count has been set to  `%s'\n", optarg);
		g_iteration = strtol(optarg,&stop_char,0);
               break;

             case 'c':
           //    printf ("cutoff_count has been set to  `%s'\n", optarg);
		g_threshold = strtol(optarg,&stop_char,0);
               break;
     
             case 'g':
           //    printf ("gussian_smoothing `%s'\n", optarg);
		g_sigma = (float)strtod (optarg, NULL);
               break;

		 case 'v':
           //    printf ("gussian_smoothing `%s'\n", optarg);
		g_filter_value = strtod (optarg, NULL);
               break;
     
             case 'f':
            //   printf ("template file name `%s'\n", optarg);
		templateFileName.assign(optarg);
		g_templateFileName = "data/"+APPLICATION+"/"+templateFileName;
		
               break;
		
 		case 'r':
           //    printf ("training file name `%s'\n", optarg);
		trainFileName.assign(optarg);
		g_trainingFileName = "data/"+APPLICATION+"/"+trainFileName;
		
               break;

 		case 'e':
            //   printf ("testing file name `%s'\n", optarg);
		testFileName.assign(optarg);
		g_testingFileName = "data/"+APPLICATION+"/"+testFileName;
               break;

		case 'm':
		g_mwu.assign(optarg);
		break;
		
		case 'l':
	//	printf ("application `%s'\n", optarg);
		APPLICATION.assign(optarg);
		g_templateFileName = "data/"+APPLICATION+"/"+templateFileName;
		g_trainingFileName = "data/"+APPLICATION+"/"+trainFileName;
		g_testingFileName = "data/"+APPLICATION+"/"+testFileName;
               break;
     
             case '?':
		cout<<"--syn"<<"\t"<<"training and tagging at the same time"<<endl;
		cout<<"--trn"<<"\t"<<"only training,(produce models in diretory model/application)"<<endl;
		cout<<"--tst"<<"\t"<<"only testing, (models has been created)"<<endl;
		cout<<"l"<<"\t"<<"specify application name"<<endl;
                cout<<"c" <<"\t"<<"cutoff_count"<<endl;
		cout<<"g"<<"\t"<<"gussian_smoothing"<<endl;
		cout<<"r"<<"\t"<<"training_file_name"<<endl;
		cout<<"e"<<"\t"<<"testing_file_name"<<endl;
		cout<<"t"<<"\t"<<"traing_method"<<endl;
		cout<<"f"<<"\t"<<"template_file"<<endl;
		cout<<"i"<<"\t"<<"iteration time"<<endl;
		cout<<"-------------------------------------------"<<endl;
		cout<<"example: me_tagging --syn -t 1 -i 500 -g 0.8 -r train.txt -e test.txt -f template.txt"<<endl;
		cout<<endl;
		cout<<"**************************************************************"<<endl;
         	exit(0);
               break;
     
             default:
               abort ();
             }
         }
     
       /* Instead of reporting `--verbose'
          and `--brief' as they are encountered,
          we report the final status resulting from them. */
       if (verbose_flag)
         puts ("verbose flag is set");
     
       /* Print any remaining command line arguments (not options). */
       if (optind < argc)
         {
           printf ("non-option ARGV-elements: ");
           while (optind < argc)
             printf ("%s ", argv[optind++]);
           putchar ('\n');
         }

	//give report of user options, the user can check his options again here!
	if(trn_flag || g_syn_flag || tst_flag)
	{
		if(g_me == 1)
		{
			printf ("ME is used \n");
		}
		if(g_crf == 1)
		{
			printf ("CRF is used \n");
			if(g_Bigram == 1)
				printf ("--Bigram model is used \n");
			else
				printf ("--Uni model is used \n");
		}
		if(g_decodingMethod == 0)
			printf ("decoding method single best \n");
		if(g_decodingMethod == 1)
			printf ("decoding method N best \n");
		if(g_decodingMethod == 2)
			printf ("decoding method FB Based on ME (no context) \n");
		if(g_decodingMethod == 3)
			printf ("decoding method FB Based on Combined models(no context) \n");
		if(g_decodingMethod == 4)
			printf ("decoding method FB Based on ME (with context) \n");
		if(g_decodingMethod == 5)
			printf ("decoding method FB Based on Combined models(with context) \n");
				
		

		
		printf ("application %s \n", APPLICATION.c_str());
		printf ("template file name %s \n", g_templateFileName.c_str());
		printf ("training file name %s \n", g_trainingFileName.c_str());
		printf ("testing file name %s \n", g_testingFileName.c_str());
		
	
		printf ("cut_off %d \n", g_threshold);
		printf ("iteration_time %d \n", g_iteration);
		printf ("gussian_smoothing %f \n", g_sigma);
	}

	cout<<"**************************************************************"<<endl;

}

void METrain(int ATrainCount = 100)
{
	CPosMaxEn cPosMaxEn;
	cPosMaxEn.TrainModel(g_trainingFileName);
	//cPosMaxEn.Continue_TrainModel(file,ATrainCount);
}


void CRFTrain(int ATrainCount = 100)
{
	CPosCRF cPosCRF;
	cPosCRF.Train(g_trainingFileName);
};

void CRFTest()
{
	
	CPosCRF cPosCRF;
	cPosCRF.TagFile(g_testingFileName);
	cout<<endl;
	cerr<<"g_templateFileName="<<g_templateFileName;
	cerr<<" c="<<g_threshold;
	cerr<<" g="<<g_sigma;
	cerr<<" "<<double(g_right*100)/g_sum<<endl;
}

void METest()
{	
	CPosMaxEn cPosMaxEn;
	if(g_decodingMethod == 0 ||g_decodingMethod == 1)
	{
		cPosMaxEn.TagFile(g_testingFileName);
	}
	if(g_decodingMethod == 2 ||g_decodingMethod == 3 ||
	g_decodingMethod == 4 ||g_decodingMethod == 5)
	{
		cPosMaxEn.FB_FilterFile(g_testingFileName);
	}
}


int main(int argc , char* argv[])
{

	/*string aa = "model/"+APPLICATION+"/aaa.txt";
	cout<<aa<<endl;

	FILE * fpEvent = fopen(aa.c_str(),"wb");
	fclose(fpEvent);
	return 1;*/

	startClk = clock();
	ParseCmd(argc,argv);

	if(g_decodingMethod == 3 || g_decodingMethod == 5)
	{
		g_combine = 1;
	}
	if(g_decodingMethod == 4 || g_decodingMethod == 5)
	{
		g_context = 1;
	}
	if(g_context == 1)
		p_dataDir ="./MODELS_CONTEXT/";
	if(g_combine== 1 || g_context == 1 )
	{
		int fadd_key= fadd_init_lib(16);
		tagger_loadfadd(p_dataDir+WORDTAGTUPLEFILE,
			p_dataDir+TAGWORDTUPLEFILE,
			p_dataDir+CONTEXTTAGTUPLEFILE,
			p_dataDir+BIGRAMTUPLEFILE,
			p_dataDir+TRIGRAMTUPLEFILE,
			p_dataDir+FOURGRAMTUPLEFILE,
			p_dataDir+PREFIXBIGRAMTUPLEFILE,
			p_dataDir+PREFIXTRIGRAMTUPLEFILE,
			p_dataDir+WORDDICTFILE,
			p_dataDir+TAGDICTFILE,
			p_dataDir+CONTEXTDICTFILE,
			p_dataDir+WORDTAGLEXICONDICTFILE);
		tagger_loadContextLabels(p_dataDir+USEDCONTEXTFILE);
	}
	
	if(g_decodingMethod == 2)
	{
		g_usedContextLabels.push_back(DUMMY_CONTEXT);
		//to make a fake context information
	} 
	
	


	/*string context_result = "context_result";
	fstream f_context;
	f_context.open (context_result.c_str(), fstream::in );

	while(!f_context.eof())
	{
		string key;
		double value;
		f_context>>key;
		f_context>>value;
		pair<string,double> item;
		item.first = key;
		item.second = value;
		g_hashContextProb.insert(item);
	}
	f_context.close();
	*/

	if(!trn_flag && !g_syn_flag&&!tst_flag)
	{
		cout<<"**********************Important Notice*************************"<<endl;
		cout<<"you should specify --me, --crf firstly"<<endl;
		cout<<"then specify --syn, --trn or --tst options secondly"<<endl;	
		cout<<"use --help options or conduct the manual to get detailed information"<<endl;
		return 0;
	}
	
	if(g_crf ==1)
	{
		if(trn_flag || g_syn_flag)
		{
			CRFTrain();
		}
		else if(tst_flag)
		{
			CRFTest();
		}
	}
	if(g_me ==1)
	{
		if(trn_flag || g_syn_flag)
		{
			METrain();
		}
		else if(tst_flag)
		{
			METest();
		}
	}	
	cout<<"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"<<endl;
	cout<<"run for "<<((double)(clock()-startClk))/CLOCKS_PER_SEC<<"seconds"<<endl;
	cout<<"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"<<endl;

	tagger_close();
	
	return 0;
}



