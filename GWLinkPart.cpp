//------------------------------------------------------------------------------
//These includes are needed for the following template code
//------------------------------------------------------------------------------
#include "GWLinkPart.hpp"
#include<iostream>
using namespace NXOpen;
using namespace NXOpen::BlockStyler;

//------------------------------------------------------------------------------
// Initialize static variables
//------------------------------------------------------------------------------
Session *(GWLinkPart::theSession) = NULL;
UI *(GWLinkPart::theUI) = NULL;
NXOpen::BlockStyler::Tree* pmiTree;// Block type: Tree Control
 
std::string GWLinkPart::strListConfig = "";
std::string GWLinkPart::strDialogConfig = "";
std::vector<Annotations::Annotation *> GWLinkPart::pmiAnnotationVector;
std::vector<ITEMDATA> GWLinkPart::itemDataVector;
std::vector<COLUMNDATA> GWLinkPart::colunmDataVector;
//------------------------------------------------------------------------------
// Constructor for NX Styler class
//------------------------------------------------------------------------------
GWLinkPart::GWLinkPart()
{
	try
	{
	

		// Initialize the NX Open C++ API environment
		GWLinkPart::theSession = NXOpen::Session::GetSession();
		GWLinkPart::theUI = UI::GetUI();
	 
		theDlxFileName = "GWLinkPart.dlx";
		theDialog = GWLinkPart::theUI->CreateDialog(theDlxFileName);
		// Registration of callback functions
		theDialog->AddApplyHandler(make_callback(this, &GWLinkPart::apply_cb));
		theDialog->AddOkHandler(make_callback(this, &GWLinkPart::ok_cb));
		theDialog->AddUpdateHandler(make_callback(this, &GWLinkPart::update_cb));
		theDialog->AddCancelHandler(make_callback(this, &GWLinkPart::cancel_cb));
		theDialog->AddFilterHandler(make_callback(this, &GWLinkPart::filter_cb));
		theDialog->AddInitializeHandler(make_callback(this, &GWLinkPart::initialize_cb));
		theDialog->AddDialogShownHandler(make_callback(this, &GWLinkPart::dialogShown_cb));
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		throw;
	}
}

//------------------------------------------------------------------------------
// Destructor for NX Styler class
//------------------------------------------------------------------------------
GWLinkPart::~GWLinkPart()
{
	if (theDialog != NULL)
	{
		delete theDialog;
		theDialog = NULL;
	}
}

extern "C" DllExport void  GW_link_part_dialog(const char *list_config, const char *dialog_config)
{
	if (!GWLinkPart::InitializeColumnDatas())
		return;
	int is_init = UF_is_initialized();
	if (!is_init)
		UF_CALL(UF_initialize());

	GWLinkPart::strListConfig = list_config;
	GWLinkPart::strDialogConfig = dialog_config;
	GWLinkPart *theGWLinkPart = NULL;
	try
	{
		GWLinkPart::InitializePmiAnnotationVector();
		theGWLinkPart = new GWLinkPart();
		// The following method shows the dialog immediately
		theGWLinkPart->Show();
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWLinkPart::theUI->NXMessageBox()->Show("Error Message33333", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	if(theGWLinkPart != NULL)
	{
		delete theGWLinkPart;
		theGWLinkPart = NULL;
	}

	if (!is_init)
		UF_CALL(UF_terminate());
}

int GWLinkPart::Show()
{
	try
	{
		theDialog->Show();
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWLinkPart::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return 0;
}

//------------------------------------------------------------------------------
//---------------------Block UI Styler Callback Functions--------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//Callback Name: initialize_cb
//------------------------------------------------------------------------------
void GWLinkPart::initialize_cb()
{
	try
	{
		UF_print_syslog(" has initialize_cb", FALSE);
		 
		groupPart = dynamic_cast<NXOpen::BlockStyler::Group*>(theDialog->TopBlock()->FindBlock("groupPart"));
		selectPart = dynamic_cast<NXOpen::BlockStyler::SelectPartFromList*>(theDialog->TopBlock()->FindBlock("selectPart"));
		groupPmi = dynamic_cast<NXOpen::BlockStyler::Group*>(theDialog->TopBlock()->FindBlock("groupPmi"));
		selectPmi = dynamic_cast<NXOpen::BlockStyler::SelectObject*>(theDialog->TopBlock()->FindBlock("pmiSelection"));
	 
		//-------------------------------------------------------------
		pmiTree = dynamic_cast<NXOpen::BlockStyler::Tree*>(theDialog->TopBlock()->FindBlock("pmiTree"));
		//pmiTree->SetOnSelectHandler(make_callback(this, &GWLinkPart::OnSelectCallback));
	//	pmiTree->SetStateIconNameHandler(make_callback(this, &GWLinkPart::StateIconNameCallback));
	  
		pmiTree->SetOnSelectHandler(make_callback(this, &GWLinkPart::OnSelectCallback));
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWLinkPart::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}
Annotations::Annotation* GWLinkPart::GetPmiAnnotation(const BlockStyler::Node *pmiNode)
{
	for (std::vector<ITEMDATA>::const_iterator itemDataVector_cit = itemDataVector.begin(); itemDataVector_cit != itemDataVector.end(); itemDataVector_cit++)
	{
		if (itemDataVector_cit->pmiNode == pmiNode)
			return itemDataVector_cit->pmiAnnotation;
	}
	return NULL;
}
void GWLinkPart::OnSelectCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int columnID, bool selected)
{
	
	UF_print_syslog("into OnSelectCallback", false);
	std::vector<TaggedObject*> pmis;
	BlockStyler::PropertyList *pmiSelectProps = selectPmi->GetProperties();
	UF_print_syslog("BlockStyler::PropertyList *pmiSelectProps = selectPmi->GetProperties();", false);
	
	try
	{
		//---- Enter your callback code here -----  
		Annotations::Annotation *pmiAnnotation = GetPmiAnnotation(node);
		UF_print_syslog("Annotations::Annotation *pmiAnnotation = GetPmiAnnotation(node);", false);
		if (pmiAnnotation)
		{
			UF_print_syslog("has one select",false);
			pmis.push_back(pmiAnnotation);
		}
		pmiSelectProps->SetTaggedObjectVector("SelectedObjects", pmis);
	}
	catch (exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWLinkPart::theUI->NXMessageBox()->Show("Error M444444essage", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	delete pmiSelectProps;
}
//------------------------------------------------------------------------------
//Callback Name: dialogShown_cb
//This callback is executed just before the dialog launch. Thus any value set 
//here will take precedence and dialog will be launched showing that value. 
//------------------------------------------------------------------------------
void GWLinkPart::dialogShown_cb()
{
	try
	{
		//---- Enter your callback code here -----
		for (int i = 0; i<(int)(colunmDataVector.size()); i++)
		{
			pmiTree->InsertColumn(i, colunmDataVector[i].columnTitle, colunmDataVector[i].columnWidth);
			pmiTree->SetColumnResizePolicy(i, Tree::ColumnResizePolicyConstantWidth);
		}
	 InitializePmiTreeList();
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWLinkPart::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}

//------------------------------------------------------------------------------
//Callback Name: apply_cb
//------------------------------------------------------------------------------
int GWLinkPart::apply_cb()
{
	int errorCode = 0;
	try
	{
		//---- Enter your callback code here -----
		LinkPartToPmi();
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		errorCode = 1;
		GWLinkPart::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return errorCode;
}
BlockStyler::Node* GWLinkPart::GetPmiNode(const Annotations::Annotation *pmiAnnotation)
{
	for (std::vector<ITEMDATA>::const_iterator itemDataVector_cit = itemDataVector.begin(); itemDataVector_cit != itemDataVector.end(); itemDataVector_cit++)
	{
		if (itemDataVector_cit->pmiAnnotation == pmiAnnotation)
			return itemDataVector_cit->pmiNode;
	}
	return NULL;
}
//------------------------------------------------------------------------------
//Callback Name: update_cb
//------------------------------------------------------------------------------
int GWLinkPart::update_cb(NXOpen::BlockStyler::UIBlock* block)
{
	std::vector<ITEMDATA>::iterator itemDataVector_it;
	Annotations::Annotation *pmiAnnotation = NULL;
	BlockStyler::Node *pmiNode = NULL;

	 
	BlockStyler::PropertyList *pmiSelectProps = selectPmi->GetProperties();
	try
	{
		std::vector<TaggedObject*> selectedObjects = pmiSelectProps->GetTaggedObjectVector("SelectedObjects");
		if (selectedObjects.size())
		{
			pmiAnnotation = dynamic_cast<Annotations::Annotation *>(selectedObjects.front());
			pmiNode = GetPmiNode(pmiAnnotation);
		}
 
		else if (block == selectPmi)
		{
			//---------Enter your code here-----------
			if (pmiNode)
			{
				pmiTree->SelectNode(pmiNode, true, true);
				pmiNode->ScrollTo(0, BlockStyler::Node::ScrollCenter);
			}
			else
			{
				pmiTree->SelectNodes(pmiTree->GetSelectedNodes(), false, true);
			}
		}
	 
	}
	catch (exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWLinkPart::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
 
	delete pmiSelectProps;
	return 0;
}

//------------------------------------------------------------------------------
//Callback Name: ok_cb
//------------------------------------------------------------------------------
int GWLinkPart::ok_cb()
{
	int errorCode = 0;
	try
	{
		errorCode = apply_cb();
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		errorCode = 1;
		GWLinkPart::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return errorCode;
}

//------------------------------------------------------------------------------
//Callback Name: cancel_Cb
//------------------------------------------------------------------------------
int GWLinkPart::cancel_cb()
{
	try
	{
		//---- Enter your callback code here -----
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWLinkPart::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return 0;
}

//------------------------------------------------------------------------------
//Callback Name: filter_cb
//------------------------------------------------------------------------------
int GWLinkPart::filter_cb(NXOpen::BlockStyler::UIBlock* block, NXOpen::TaggedObject* selectObject)
{
	if (block == selectPmi)
	{
		Annotations::Annotation *pmiAnnotation(dynamic_cast<Annotations::Annotation *>(selectObject));
		if (pmiAnnotation)
		{
			if (NULL == GW_find_associate_part(pmiAnnotation))
				return UF_UI_SEL_ACCEPT;
		}
		return UF_UI_SEL_REJECT;
	}
	return(UF_UI_SEL_ACCEPT);
}

//------------------------------------------------------------------------------
//Function Name: GetBlockProperties
//Description: Returns the propertylist of the specified BlockID
//------------------------------------------------------------------------------
PropertyList* GWLinkPart::GetBlockProperties(const char *blockID)
{
	return theDialog->GetBlockProperties(blockID);
}

void GWLinkPart::LinkPartToPmi()
{
	 
	BlockStyler::PropertyList *selectPartProps = selectPart->GetProperties();
	UF_print_syslog("11111111111111111111111111111",false);
	std::vector<TaggedObject*> selectedParts = selectPartProps->GetTaggedObjectVector("SelectedObjects");
	UF_print_syslog("2222222222222222222222222222", false);
	delete selectPartProps;
	UF_print_syslog("3333333333333333333333333333", false);
	BlockStyler::PropertyList *selectPmiProps = selectPmi->GetProperties();
	UF_print_syslog("44444444444444444444444444444", false);
	std::vector<TaggedObject*> selectedPmis = selectPmiProps->GetTaggedObjectVector("SelectedObjects");
	UF_print_syslog("555555555555555555555555555555", false);
	delete selectPmiProps;
	UF_print_syslog("6666666666666666666666666666666666666", false);
	if (0 == selectedParts.size() || 0 == selectedPmis.size())
		return;
	UF_print_syslog("7777777777777777777777777777", false);
	Part *part = dynamic_cast<Part *>(selectedParts[0]);
	UF_print_syslog("8888888888888888888888888888888", false);
	Annotations::Annotation *pmiAnnotation = dynamic_cast<Annotations::Annotation *>(selectedPmis[0]);
	UF_print_syslog("999999999999999999999999999999", false);
	
	if (NULL == part || NULL == pmiAnnotation)
		return;
	 
	try
	{
		UF_print_syslog("00000000000000000000000000000000000000", false);
	 	GW_load_part(part->GetTag());	
		UF_print_syslog("qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq", false);
		if (GW_is_find_attribute(part, GW_PMI_HANDLE_NAME, NXObject::AttributeTypeString))
		{
			UF_print_syslog("wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww", false);
			uc1601("选择的组件已经关联至PMI,请重新选择!", 1);
			return;
		}
		UF_print_syslog("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee", false);
		if (!IsEqualPmiText(part, pmiAnnotation, strListConfig.c_str(), strDialogConfig.c_str()))
			return;
		UF_print_syslog("rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr", false);
	part->SetAttribute(GW_PMI_HANDLE_NAME, UF_TAG_ask_handle_of_tag(pmiAnnotation->GetTag()));
	UF_print_syslog("tttttttttttttttttttttttttttttttttttt", false);
	part->SetAttribute("是否关联", "已关联");
	UF_print_syslog("yyyyyyyyyyyyyyyyyyyyyyyyyyyy", false);
		SetListColumnDatas(part, pmiAnnotation, strListConfig.c_str());
		UF_print_syslog("uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu", false);
		 
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	 
}

bool GWLinkPart::SetListColumnDatas(Part *part, Annotations::Annotation *pmiAnnotation, const char *list_config)
{
	std::vector<NXString>	stringArray;
	char		buffer[GW_MAX_STRING_LEN], attr_title[UF_ATTR_MAX_TITLE_LEN+1], attr_value[UF_ATTR_MAX_STRING_LEN+1];

	tag_t part_tag = part->GetTag();
	tag_t pmi_tag = pmiAnnotation->GetTag();
	FILE *fp = fopen(list_config, "r");
	if (fp == NULL)
		return false;
	while (NULL != fgets(buffer, GW_MAX_STRING_LEN, fp))
	{
		if (GW_split_strings(buffer, ";", stringArray) >= 3)
		{
			strcpy(attr_title, stringArray[0].GetLocaleText());
			if (GW_ask_object_attribute(part_tag, attr_title, attr_value))
			{
				GW_set_object_attribute(pmi_tag, attr_title, attr_value);
			}
		}
	}
	fclose(fp);
	return true;
}

bool GWLinkPart::IsEqualPmiText(Part *part, Annotations::Annotation *pmiAnnotation, const char *list_config, const char *dialog_config)
{
	std::vector<NXString>	splitArray;
	std::string	leftString, rightString;
	char		buffer[GW_MAX_STRING_LEN], attr_title[UF_ATTR_MAX_TITLE_LEN+1], attr_value[UF_ATTR_MAX_STRING_LEN+1], pmi_text[UF_ATTR_MAX_STRING_LEN+1];
	int			pmiTextColumn = 5;

	tag_t part_tag = part->GetTag();
	tag_t pmi_tag = pmiAnnotation->GetTag();
	FILE *fp = fopen(dialog_config, "r");
	if (fp == NULL)
		return false;
	while (NULL != fgets(buffer, GW_MAX_STRING_LEN, fp))
	{
		if (2 == GW_split_strings(buffer, "=", splitArray))
		{
			leftString = splitArray[0].GetLocaleText();
			rightString = splitArray[1].GetLocaleText();
			GW_trim(leftString);
			GW_trim(rightString);
			if (rightString.empty())
				continue;
			if ("PMI_TEXT_COLUMN" == leftString)
			{
				pmiTextColumn = atoi(rightString.c_str());
			}
		}
	}
	fclose(fp);

	fp = fopen(list_config, "r");
	if (fp == NULL)
		return false;
	int columnIndex = 0;
	while (NULL != fgets(buffer, GW_MAX_STRING_LEN, fp))
	{
		if (GW_split_strings(buffer, ";", splitArray) >= 3)
		{
			if (columnIndex == pmiTextColumn)
			{
				strcpy(attr_title, splitArray[0].GetLocaleText());
				if (GW_ask_object_attribute(part_tag, attr_title, attr_value))
				{
					NXString annotationText = GW_ask_pmi_annotation_text(pmiAnnotation);
					strcpy(pmi_text, annotationText.GetLocaleText());
					fclose(fp);
					if (0 == strcmp(pmi_text, attr_value))
					{
						return true;
					}
					else
					{
						char message[GW_MAX_STRING_LEN] = "";
						sprintf(message, "PMI文本\"%s\"与部件信息\"%s\"不一致，是否关联?", pmi_text, attr_value);
						if (1 == GW_yes_or_no_box("选择的部件没有PMI文本信息，是否关联?", "问题"))
							return true;
						return false;
					}
				}
			}
			columnIndex++;
		}
	}
	fclose(fp);

	if (1 == GW_yes_or_no_box("选择的部件没有PMI文本信息，是否关联?", "问题"))
		return true;
	return false;
}
//-------------------------------------------------------------------
NXString GWLinkPart::StateIconNameCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int state)
{
	NXOpen::NXString stateIcon;
	try
	{
		//---- Enter your callback code here -----
		switch (state - GW_START_DIMENSION_ICON)
		{
		case 0:
			stateIcon = "csys_view";
			break;
		case UF_dim_horizontal_subtype:
			stateIcon = "horizontal_dim";
			break;
		case UF_dim_vertical_subtype:
			stateIcon = "vertical_dim";
			break;
		case UF_dim_parallel_subtype:
			stateIcon = "parallel_dim";
			break;
		case UF_dim_cylindrical_subtype:
			stateIcon = "cylindrical_dim";
			break;
		case UF_dim_perpendicular_subtype:
			stateIcon = "perpendicular_dim";
			break;
		case UF_dim_angular_minor_subtype:
			stateIcon = "angular_dim";
			break;
		case UF_dim_angular_major_subtype:
			stateIcon = "dim_angle_major";
			break;
		case UF_dim_arc_length_subtype:
			stateIcon = "arclength_dim";
			break;
		case UF_dim_radius_subtype:
			stateIcon = "radius_dim";
			break;
		case UF_dim_diameter_subtype:
			stateIcon = "diameter_dim";
			break;
		case UF_dim_hole_subtype:
			stateIcon = "hole_dim";
			break;
		case UF_dim_conc_circle_subtype:
			stateIcon = "concentric_dim";
			break;
		case UF_dim_ordinate_horiz_subtype:
			stateIcon = "ordinate_dim";
			break;
		case UF_dim_ordinate_vert_subtype:
			stateIcon = "ordinate_dim";
			break;
		case UF_dim_assorted_parts_subtype:
			stateIcon = "autoinherit_dim";
			break;
		case UF_dim_folded_radius_subtype:
			stateIcon = "foldradius_dim";
			break;
		case UF_dim_chain_dimensions_subtype:
			stateIcon = "dim_chain_horiz";
			break;
		case UF_dim_ordinate_origin_subtype:	//not used
			break;
		case UF_dim_perimeter_subtype:
			stateIcon = "perimeter_dim";
			break;
		case UF_dim_chamfer_subtype:
			stateIcon = "chamfer_dim";
			break;
		}
		if (GW_SURFACE_FINISH_ICON == state)
		{
			stateIcon = "surface_finish";
		}
		if (GW_GDT_SYMBOL_ICON == state)
		{
			stateIcon = "autoinherit_gdt";
		}
	}
	catch (exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWLinkPart::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return stateIcon;
}
void GWLinkPart::InitializePmiTreeList()
{
	Part *workPart(theSession->Parts()->Work());
 
 
		//---- Enter your callback code here -----

	 
	
	BlockStyler::PropertyList *pmiSelectProps = selectPmi->GetProperties();

	std::vector<TaggedObject*> pmis;
	try
	{
 	pmiSelectProps->SetTaggedObjectVector("SelectedObjects", pmis);
 
	 
		BlockStyler::Node *rootNote = pmiTree->RootNode();
		while (rootNote)
		{
			pmiTree->DeleteNode(rootNote);
			rootNote = pmiTree->RootNode();
		}
		itemDataVector.clear();
	 
		 
	   	

		std::vector<NXOpen::NXString> itemList ;
		BlockStyler::Node *viewNode = pmiTree->CreateNode("root");
		pmiTree->InsertNode(viewNode, NULL, NULL, NXOpen::BlockStyler::Tree::NodeInsertOptionSort);
		for (std::vector<Annotations::Annotation *>::iterator pmiAnnotationVector_it = pmiAnnotationVector.begin(); pmiAnnotationVector_it != pmiAnnotationVector.end(); pmiAnnotationVector_it++)
		{
		
			Annotations::Annotation *pmiAnnotation = *pmiAnnotationVector_it;
			AddPmiToViewNode(viewNode, pmiAnnotation);
		
		}
		
	
		
		/*
			pmiListBox->SetListItems(itemList);
	
		std::vector<NXOpen::View *> layoutViews = workPart->Layouts()->Current()->GetViews();
			for (std::vector<NXOpen::View *>::iterator layoutViews_it = layoutViews.begin(); layoutViews_it != layoutViews.end(); layoutViews_it++)
			{
				UF_print_syslog("has view222",false);
			//	BlockStyler::Node *viewNode = pmiTree->CreateNode("root");
 
			//	pmiTree->InsertNode(viewNode, NULL, NULL, NXOpen::BlockStyler::Tree::NodeInsertOptionSort);
	 	//	  	InsertViewNode(dynamic_cast<NXOpen::ModelingView *>(*layoutViews_it));
			}
 		*/
	}
	catch (exception& ex)
	{
	 
		UF_print_syslog("InitializePmiTreeList has error",false);
		;
	 
		//---- Enter your exception handling code here -----
		GWLinkPart::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
//	delete pmiSelectProps;
//	delete viewRadioProps;
}
void GWLinkPart::InitializePmiAnnotationVector()
{
	Session *theSession = NXOpen::Session::GetSession();
	Part	*workPart(theSession->Parts()->Work());
	UI		*theUI = UI::GetUI();
	int		type, subtype;

	pmiAnnotationVector.clear();
	try
	{
		NXOpen::Annotations::PmiCollection *pmiCollection = workPart->PmiManager()->Pmis();
		for (NXOpen::Annotations::PmiCollection::iterator pmiCollection_it = pmiCollection->begin(); pmiCollection_it != pmiCollection->end(); pmiCollection_it++)
		{
			std::vector<NXOpen::Annotations::Annotation *> instances = (*pmiCollection_it)->GetDisplayInstances();
			for (std::vector<NXOpen::Annotations::Annotation *>::iterator instances_it = instances.begin(); instances_it != instances.end(); instances_it++)
			{
				
				Annotations::Annotation *pmiAnnotation = dynamic_cast<Annotations::Annotation *>(*instances_it);
				if (NULL == pmiAnnotation)
					continue;
			 	pmiAnnotationVector.push_back(pmiAnnotation);
				/*
				UF_print_syslog("----------------------------", false);
				tag_t pmi_tag = (*instances_it)->GetTag();
				Annotations::Dimension *pmiDimension = dynamic_cast<Annotations::Dimension *>(pmiAnnotation);
				if (pmiDimension)
				{
					UF_OBJ_ask_type_and_subtype(pmi_tag, &type, &subtype);
					UF_print_syslog("-------------1---------------", false);
					 
					if (UF_dimension_type == type && UF_dim_ordinate_origin_subtype != subtype)
					{
						pmiAnnotationVector.push_back(pmiAnnotation);
						continue;
					}
				}
				UF_print_syslog("sssssssssssss", false);
				UF_print_syslog(const_cast<char*>(typeid(pmiAnnotation).name()),false);
				
			//	Annotations::Note *pmiNote = dynamic_cast<Annotations::Note *>(pmiAnnotation);
				Annotations::SurfaceFinish *pmiSurfaceFinish = dynamic_cast<Annotations::SurfaceFinish *>(pmiAnnotation);
				if (pmiSurfaceFinish)
				{
					UF_print_syslog("-------------2---------------", false);
					pmiAnnotationVector.push_back(pmiAnnotation);
					continue;
				}
				Annotations::Fcf *fcf = dynamic_cast<Annotations::Fcf *>(pmiAnnotation);
				if (fcf)
				{
					UF_print_syslog("-------------3---------------", false);
					pmiAnnotationVector.push_back(pmiAnnotation);
					continue;
				}
				*/
			}
		}
	}
	catch (exception& ex)
	{
		//---- Enter your exception handling code here -----
		theUI->NXMessageBox()->Show("Error InitializePmiAnnotationVector  Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}

void GWLinkPart::InsertViewNode(NXOpen::ModelingView *modelingView)
{
	if (NULL == modelingView)
		return;
	try
	{
	


		//---- Enter your callback code here -----
		/*
		BlockStyler::Node *viewNode = pmiTree->CreateNode(modelingView->Name());
 
		
		pmiTree->InsertNode(viewNode, NULL, NULL, NXOpen::BlockStyler::Tree::NodeInsertOptionSort);
	
		viewNode->SetState(GW_START_DIMENSION_ICON);
		viewNode->ScrollTo(0, BlockStyler::Node::ScrollCenter);
	
	
		for (std::vector<Annotations::Annotation *>::iterator pmiAnnotationVector_it = pmiAnnotationVector.begin(); pmiAnnotationVector_it != pmiAnnotationVector.end(); pmiAnnotationVector_it++)
		{
			Annotations::Annotation *pmiAnnotation = *pmiAnnotationVector_it;
			std::vector<NXOpen::View *> pmiViews = pmiAnnotation->GetViews();
			if (0 == pmiViews.size())
			{
			 AddPmiToViewNode(viewNode, pmiAnnotation);
			}
			else
			{
				for (std::vector<NXOpen::View *>::const_iterator pmiViews_cit = pmiViews.begin(); pmiViews_cit != pmiViews.end(); pmiViews_cit++)
				{
					if ((*pmiViews_cit) == modelingView)
					{
				 	AddPmiToViewNode(viewNode, pmiAnnotation);
					}
				}
			}
		}
	
		viewNode->Expand(BlockStyler::Node::ExpandOptionExpand);
		*/
	}
	catch (exception& ex)
	{
		//---- Enter your exception handling code here -----
		UF_print_syslog(" InsertViewNode has error",false);
		GWLinkPart::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}

void GWLinkPart::AddPmiToViewNode(BlockStyler::Node *viewNode, Annotations::Annotation *pmiAnnotation)
{
	int			type, subtype;
	ITEMDATA	itemData;

	if (NULL == viewNode)
		return;
	try
	{
		itemData.pmiAnnotation = pmiAnnotation;
		itemData.pmiNode = pmiTree->CreateNode(AskPmiTreeName(pmiAnnotation));
		pmiTree->InsertNode(itemData.pmiNode, viewNode, NULL, NXOpen::BlockStyler::Tree::NodeInsertOptionSort);
		itemDataVector.push_back(itemData);
		for (int i = 0; i<(int)(colunmDataVector.size()); i++)
		{
			itemData.pmiNode->SetColumnDisplayText(i, GW_ask_pmi_annotation_text(pmiAnnotation));
		 
		}
		Annotations::Dimension *pmiDimension = dynamic_cast<Annotations::Dimension *>(itemData.pmiAnnotation);
		if (pmiDimension)
		{
			UF_OBJ_ask_type_and_subtype(itemData.pmiAnnotation->GetTag(), &type, &subtype);
			itemData.pmiNode->SetState(GW_START_DIMENSION_ICON + subtype);
		}
		Annotations::SurfaceFinish *surfaceFinish = dynamic_cast<Annotations::SurfaceFinish *>(itemData.pmiAnnotation);
		if (surfaceFinish)
		{
			itemData.pmiNode->SetState(GW_SURFACE_FINISH_ICON);
		}
		Annotations::Fcf *fcf = dynamic_cast<Annotations::Fcf *>(itemData.pmiAnnotation);
		if (fcf)
		{
			itemData.pmiNode->SetState(GW_GDT_SYMBOL_ICON);
		}
	}
	catch (exception& ex)
	{
		UF_print_syslog(" AddPmiToViewNode has error", false);
		//---- Enter your exception handling code here -----
		GWLinkPart::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}

NXString GWLinkPart::AskPmiTreeName(Annotations::Annotation *pmiAnnotation)
{
	char		object_handle[UF_ATTR_MAX_STRING_LEN];
	NXString	strTreeName;

	if (GW_find_associate_part(pmiAnnotation))
	{
		strTreeName += "√ ";
	}
	else
	{
		strTreeName += "? ";
	}
	if (GW_is_find_attribute(pmiAnnotation, GW_UNLOCK_PMI_TEXT, NXObject::AttributeTypeString))
	{
		strTreeName += "<";
		strTreeName += GW_ask_annotation_name(pmiAnnotation);
		strTreeName += ">";
	}
	else
	{
		strTreeName += GW_ask_annotation_name(pmiAnnotation);
	}
	return strTreeName;
}
bool GWLinkPart::InitializeColumnDatas()
{
	std::vector<NXString>	stringArray;
	COLUMNDATA	columnData;
	char		buffer[GW_MAX_STRING_LEN];

	colunmDataVector.clear();
 
  
			columnData.columnTitle = "编号";
			columnData.columnWidth = 125;
			columnData.columnDefaults = "";
			colunmDataVector.push_back(columnData);
 
 
 
	return true;
}