#include <time.h>

//------------------------------------------------------------------------------UpdateDefaultValues
//These includes are needed for the following template code
//------------------------------------------------------------------------------
#include "GWDimList.hpp"
using namespace NXOpen;
using namespace NXOpen::BlockStyler;

//------------------------------------------------------------------------------
// Initialize static variables
//------------------------------------------------------------------------------
Session *(GWDimList::theSession) = NULL;
UI *(GWDimList::theUI) = NULL;

std::vector<COLUMNDATA> GWDimList::colunmDataVector;
std::vector<Annotations::Annotation *> GWDimList::pmiAnnotationVector;
Annotations::Annotation *GWDimList::currentAnnotation = NULL;
std::vector<std::string> GWDimList::ugmgrFolderPath;
std::vector<ITEMDATA> GWDimList::itemDataVector;
int GWDimList::pmiTextColumn = 5;
std::string GWDimList::strPartType = "";
bool GWDimList::saveData = false;
//------------------------------------------------------------------------------
// Constructor for NX Styler class
//------------------------------------------------------------------------------
GWDimList::GWDimList()
{
	try
	{
        // Initialize the NX Open C++ API environment
        GWDimList::theSession = NXOpen::Session::GetSession();
        GWDimList::theUI = UI::GetUI();
        theDlxFileName = "GWDimList.dlx";
        theDialog = GWDimList::theUI->CreateDialog(theDlxFileName);
        // Registration of callback functions
		theDialog->AddApplyHandler(make_callback(this, &GWDimList::apply_cb));
		theDialog->AddOkHandler(make_callback(this, &GWDimList::ok_cb));
		theDialog->AddUpdateHandler(make_callback(this, &GWDimList::update_cb));
		theDialog->AddCancelHandler(make_callback(this, &GWDimList::cancel_cb));
        theDialog->AddFilterHandler(make_callback(this, &GWDimList::filter_cb));
        theDialog->AddInitializeHandler(make_callback(this, &GWDimList::initialize_cb));
        theDialog->AddFocusNotifyHandler(make_callback(this, &GWDimList::focusNotify_cb));
        theDialog->AddKeyboardFocusNotifyHandler(make_callback(this, &GWDimList::keyboardFocusNotify_cb));
        theDialog->AddDialogShownHandler(make_callback(this, &GWDimList::dialogShown_cb));
		 
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
		throw;
	}
}

//------------------------------------------------------------------------------
// Destructor for NX Styler class
//------------------------------------------------------------------------------
GWDimList::~GWDimList()
{
	if (theDialog != NULL)
	{
		delete theDialog;
		theDialog = NULL;
	}
}

extern "C" DllExport void  GW_dim_list_dialog(const char *list_config, const char *dialog_config)
{
	if (!GWDimList::InitializeColumnDatas(list_config))
		return;
	if (!GWDimList::InitializeDialogDefaults(dialog_config))
		return;
	int is_init = UF_is_initialized();
	if (!is_init)
		UF_CALL(UF_initialize());

#ifndef DEBUG
	UF_CALL(UF_UGMGR_initialize(0, NULL));
#endif // DEBUG

	GWDimList *theGWDimList = NULL;
	try
	{
		GWDimList::InitializePmiAnnotationVector();
		theGWDimList = new GWDimList();
		// The following method shows the dialog immediately
		theGWDimList->Show();
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	if(theGWDimList != NULL)
	{
		delete theGWDimList;
		theGWDimList = NULL;
	}
	if (GWDimList::saveData)
	{
		for (std::vector<ITEMDATA>::const_iterator itemDataVector_cit=GWDimList::itemDataVector.begin(); itemDataVector_cit != GWDimList::itemDataVector.end(); itemDataVector_cit++)
		{
			if (itemDataVector_cit->pmiAnnotation)
			{
				for (int i=0; i<(int)(itemDataVector_cit->attributeTitles.size()); i++)
				{
					itemDataVector_cit->pmiAnnotation->SetAttribute(itemDataVector_cit->attributeTitles[i], itemDataVector_cit->attributeValues[i]);
				}
				if (itemDataVector_cit->isUnlockText)
					GW_set_object_attribute(itemDataVector_cit->pmiAnnotation->GetTag(), GW_UNLOCK_PMI_TEXT, "1");
				else
					GW_delete_object_attribute(itemDataVector_cit->pmiAnnotation->GetTag(), GW_UNLOCK_PMI_TEXT);
				if (itemDataVector_cit->isBreakPart)
				{
					Part *part = GW_find_associate_part(itemDataVector_cit->pmiAnnotation);
					if (part)
					{
						GW_delete_object_attribute(part->GetTag(), GW_PMI_HANDLE_NAME);
					}
				}
			}
		}
	}
	GWDimList::itemDataVector.clear();
	GWDimList::pmiAnnotationVector.clear();
	GWDimList::currentAnnotation = NULL;

#ifndef DEBUG
	UF_UGMGR_terminate();
#endif // DEBUG

	if (!is_init)
		UF_CALL(UF_terminate());
}

extern "C" DllExport void  ufusr(char *param, int *retcod, int param_len)
{
	// Initialize the API environment
	if (0 != UF_CALL(UF_initialize()))
		return;

	UF_terminate();
}

extern "C" DllExport int ufusr_ask_unload()
{
	//return (int)Session::LibraryUnloadOptionExplicitly;
	return (int)Session::LibraryUnloadOptionImmediately;
	//return (int)Session::LibraryUnloadOptionAtTermination;
}

//------------------------------------------------------------------------------
// Following method cleanup any housekeeping chores that may be needed.
// This method is automatically called by NX.
//------------------------------------------------------------------------------
extern "C" DllExport void ufusr_cleanup(void)
{
	try
	{
		//---- Enter your callback code here -----
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}

int GWDimList::Show()
{
	try
	{
		theDialog->Show();
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return 0;
}

//------------------------------------------------------------------------------
//---------------------Block UI Styler Callback Functions--------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//Callback Name: initialize_cb
//------------------------------------------------------------------------------
void GWDimList::initialize_cb()
{
	try
	{
		groupSelect = dynamic_cast<NXOpen::BlockStyler::Group*>(theDialog->TopBlock()->FindBlock("groupSelect"));
		viewRadio = dynamic_cast<NXOpen::BlockStyler::Enumeration*>(theDialog->TopBlock()->FindBlock("viewRadio"));
		pmiSelection = dynamic_cast<NXOpen::BlockStyler::SelectObject*>(theDialog->TopBlock()->FindBlock("pmiSelection"));
		groupList = dynamic_cast<NXOpen::BlockStyler::Group*>(theDialog->TopBlock()->FindBlock("groupList"));
		labelTitle = dynamic_cast<NXOpen::BlockStyler::Label*>(theDialog->TopBlock()->FindBlock("labelTitle"));
		pmiTree = dynamic_cast<NXOpen::BlockStyler::Tree*>(theDialog->TopBlock()->FindBlock("pmiTree"));
		//------------------------------------------------------------------------------
		//Registration of Treelist specific callbacks
		//------------------------------------------------------------------------------
		//pmiTree->SetOnExpandHandler(make_callback(this, &GWDimList::OnExpandCallback));
		//pmiTree->SetOnInsertColumnHandler(make_callback(this, &GWDimList::OnInsertColumnCallback));
		//pmiTree->SetOnInsertNodeHandler(make_callback(this, &GWDimList::OnInsertNodeCallback));
		//pmiTree->SetOnDeleteNodeHandler(make_callback(this, &GWDimList::OnDeleteNodeCallback));
		//pmiTree->SetOnPreSelectHandler(make_callback(this, &GWDimList::OnPreSelectCallback));
		pmiTree->SetOnSelectHandler(make_callback(this, &GWDimList::OnSelectCallback));
		//pmiTree->SetOnStateChangeHandler(make_callback(this, &GWDimList::OnStateChangeCallback));
		//pmiTree->SetToolTipTextHandler(make_callback(this, &GWDimList::ToolTipTextCallback));
		//pmiTree->SetColumnSortHandler(make_callback(this, &GWDimList::ColumnSortCallback));
		pmiTree->SetStateIconNameHandler(make_callback(this, &GWDimList::StateIconNameCallback));
		//pmiTree->SetOnBeginLabelEditHandler(make_callback(this, &GWDimList::OnBeginLabelEditCallback));
		//pmiTree->SetOnEndLabelEditHandler(make_callback(this, &GWDimList::OnEndLabelEditCallback));
		pmiTree->SetOnEditOptionSelectedHandler(make_callback(this, &GWDimList::OnEditOptionSelectedCallback));
		pmiTree->SetAskEditControlHandler(make_callback(this, &GWDimList::AskEditControlCallback));
		pmiTree->SetOnMenuHandler(make_callback(this, &GWDimList::OnMenuCallback));
		pmiTree->SetOnMenuSelectionHandler(make_callback(this, &GWDimList::OnMenuSelectionCallback));
		//pmiTree->SetIsDropAllowedHandler(make_callback(this, &GWDimList::IsDropAllowedCallback));
		//pmiTree->SetIsDragAllowedHandler(make_callback(this, &GWDimList::IsDragAllowedCallback));
		//pmiTree->SetOnDropHandler(make_callback(this, &GWDimList::OnDropCallback));
		//pmiTree->SetOnDropMenuHandler(make_callback(this, &GWDimList::OnDropMenuCallback));
		//pmiTree->SetOnDefaultActionHandler(make_callback(this, &GWDimList::OnDefaultAction));
		//------------------------------------------------------------------------------
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}

//------------------------------------------------------------------------------
//Callback Name: dialogShown_cb
//This callback is executed just before the dialog launch. Thus any value set 
//here will take precedence and dialog will be launched showing that value. 
//------------------------------------------------------------------------------
void GWDimList::dialogShown_cb()
{
	try
	{
		//---- Enter your callback code here -----
		//Insert Columns  
		saveData = false;
		isModified = false;
		std::vector<NXOpen::Selection::MaskTriple> selectionMaskVector;
		NXOpen::Selection::MaskTriple selectionMask[2];
		selectionMask[0].Type = UF_dimension_type;
		selectionMask[0].Subtype = UF_all_subtype;
		selectionMask[0].SolidBodySubtype = UF_UI_SEL_NOT_A_FEATURE;
		selectionMaskVector.push_back(selectionMask[0]);
		selectionMask[1].Type = UF_drafting_entity_type;
		selectionMask[1].Subtype = UF_all_subtype;
		selectionMask[1].SolidBodySubtype = UF_UI_SEL_NOT_A_FEATURE;
		selectionMaskVector.push_back(selectionMask[1]);
		BlockStyler::PropertyList *pmiSelectProps = pmiSelection->GetProperties();
		pmiSelectProps->SetSelectionFilter("SelectionFilter", NXOpen::Selection::SelectionActionClearAndEnableSpecific, selectionMaskVector);
		delete pmiSelectProps;
		for (int i=0; i<(int)(colunmDataVector.size()); i++)
		{
			pmiTree->InsertColumn(i, colunmDataVector[i].columnTitle, colunmDataVector[i].columnWidth);
			pmiTree->SetColumnResizePolicy(i, Tree::ColumnResizePolicyConstantWidth);
		}

		InitializePmiTreeList();
		SelectCurrentAnnotation();
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}

//------------------------------------------------------------------------------
//Callback Name: apply_cb
//------------------------------------------------------------------------------
int GWDimList::apply_cb()
{
	std::vector<NXString>	nameArray;
	char	part_number[UF_UGMGR_PARTNO_SIZE+1];
	int		count, errorCode = 0;

#ifndef DEBUG
	UF_UGMGR_tag_t find_folder = GW_ugmgr_find_folder(ugmgrFolderPath);
	UF_CALL(UF_UGMGR_set_default_folder(find_folder));
#endif // DEBUG

	GW_ask_part_names_in_session(nameArray);
	RemoveUnusedComponents();
	try
	{
		//---- Enter your callback code here -----
		for (std::vector<ITEMDATA>::const_iterator itemDataVector_cit=itemDataVector.begin(); itemDataVector_cit != itemDataVector.end(); itemDataVector_cit++)
		{
			if (NULL == itemDataVector_cit->pmiAnnotation)
				continue;
			for (int i=1; i<(int)(colunmDataVector.size()); i++)
			{
				itemDataVector_cit->pmiAnnotation->SetAttribute(colunmDataVector[i].columnTitle, itemDataVector_cit->pmiNode->GetColumnDisplayText(i));
			}
			if (IsEmptyItemTexts(itemDataVector_cit->pmiNode))
				continue;

			GW_delete_object_attribute(itemDataVector_cit->pmiAnnotation->GetTag(), GW_BREAK_ASSOC_PART);
			Part *part = GW_find_associate_part(itemDataVector_cit->pmiAnnotation);
			if (NULL == part)
			{
				GW_ask_new_part_number(nameArray, part_number);
				part = CreateAssociatePart(part_number, itemDataVector_cit->pmiAnnotation);
				nameArray.push_back(part_number);
			}
			if (part)
			{
				GW_load_part(part->GetTag());
				for (int i=1; i<(int)(colunmDataVector.size()); i++)
				{
					part->SetAttribute(colunmDataVector[i].columnTitle, itemDataVector_cit->pmiNode->GetColumnDisplayText(i).GetLocaleText());
				}
			}
		}

#ifndef DEBUG
		tag_t	*part_list = NULL;
		int		*error_list = NULL;
		UF_PART_save_all(&count, &part_list, &error_list);
		UF_free(part_list);
		UF_free(error_list);
#endif // DEBUG

	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		errorCode = 1;
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return errorCode;
}

//------------------------------------------------------------------------------
//Callback Name: update_cb
//------------------------------------------------------------------------------
int GWDimList::update_cb(NXOpen::BlockStyler::UIBlock* block)
{
	std::vector<ITEMDATA>::iterator itemDataVector_it;
	Annotations::Annotation *pmiAnnotation = NULL;
	BlockStyler::Node *pmiNode = NULL;

	BlockStyler::PropertyList *viewRadioProps = viewRadio->GetProperties();
	BlockStyler::PropertyList *pmiSelectProps = pmiSelection->GetProperties();
	try
	{
		std::vector<TaggedObject*> selectedObjects = pmiSelectProps->GetTaggedObjectVector("SelectedObjects");
		if (selectedObjects.size())
		{
			pmiAnnotation= dynamic_cast<Annotations::Annotation *>(selectedObjects.front());
			pmiNode = GetPmiNode(pmiAnnotation);
		}
		if(block == viewRadio)
		{
			//---------Enter your code here-----------
			InitializePmiTreeList();
		}
		else if(block == pmiSelection)
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
		else if(block == labelTitle)
		{
			//---------Enter your code here-----------
		}
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	delete viewRadioProps;
	delete pmiSelectProps;
	return 0;
}

//------------------------------------------------------------------------------
//Callback Name: ok_cb
//------------------------------------------------------------------------------
int GWDimList::ok_cb()
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
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return errorCode;
}

//------------------------------------------------------------------------------
//Callback Name: cancel_Cb
//------------------------------------------------------------------------------
int GWDimList::cancel_cb()
{
	try
	{
		//---- Enter your callback code here -----
		if (isModified)
		{
			if (1 == GW_yes_or_no_box("是否保存修改的信息?", "问题"))
			{
				for (std::vector<ITEMDATA>::iterator itemDataVector_it=itemDataVector.begin(); itemDataVector_it != itemDataVector.end(); itemDataVector_it++)
				{
					itemDataVector_it->attributeTitles.clear();
					itemDataVector_it->attributeValues.clear();
					if (itemDataVector_it->pmiAnnotation)
					{
						for (int i=1; i<(int)(colunmDataVector.size()); i++)
						{
							itemDataVector_it->attributeTitles.push_back(colunmDataVector[i].columnTitle);
							itemDataVector_it->attributeValues.push_back(itemDataVector_it->pmiNode->GetColumnDisplayText(i));
						}
						itemDataVector_it->isBreakPart = GW_is_find_attribute(itemDataVector_it->pmiAnnotation, GW_BREAK_ASSOC_PART, NXObject::AttributeTypeString);
						itemDataVector_it->isUnlockText = GW_is_find_attribute(itemDataVector_it->pmiAnnotation, GW_UNLOCK_PMI_TEXT, NXObject::AttributeTypeString);
					}
				}
				saveData = true;
			}
		}
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return 0;
}

//------------------------------------------------------------------------------
//Callback Name: filter_cb
//------------------------------------------------------------------------------
int GWDimList::filter_cb(NXOpen::BlockStyler::UIBlock* block, NXOpen::TaggedObject* selectObject)
{
	try
	{
		if (block == pmiSelection)
		{
			Annotations::Annotation *pmiAnnotation(dynamic_cast<Annotations::Annotation *>(selectObject));
			if (pmiAnnotation)
			{
				BlockStyler::Node* pmiNode = GWDimList::GetPmiNode(pmiAnnotation);
				if (pmiNode)
				{
					return UF_UI_SEL_ACCEPT;
				}
			}
		}
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}

	return(UF_UI_SEL_REJECT);
}

//------------------------------------------------------------------------------
//Callback Name: focusNotify_cb
//This callback is executed when any block (except the ones which receive keyboard entry such as Integer block) receives focus.
//------------------------------------------------------------------------------
void GWDimList::focusNotify_cb(NXOpen::BlockStyler::UIBlock* block, bool focus)
{
	try
	{
		//---- Enter your callback code here -----
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}

//------------------------------------------------------------------------------
//Callback Name: keyboardFocusNotify_cb
//This callback is executed when block which can receive keyboard entry, receives the focus.
//------------------------------------------------------------------------------
void GWDimList::keyboardFocusNotify_cb(NXOpen::BlockStyler::UIBlock* block, bool focus)
{
	try
	{
		//---- Enter your callback code here -----
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}
//------------------------------------------------------------------------------
//Treelist specific callbacks
//------------------------------------------------------------------------------
//void GWDimList::OnExpandCallback (NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node)
//{
//}

//void GWDimList::OnInsertColumnCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int columnID)
//{
//}

//void GWDimList::OnInsertNodeCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node)
//{
//}

//void GWDimList::OnDeleteNodeCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node)
//{
//}

//void GWDimList::OnPreSelectCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int ID, bool selected)
//{
//}

void GWDimList::OnSelectCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int columnID, bool selected)
{
	std::vector<TaggedObject*> pmis;
	BlockStyler::PropertyList *pmiSelectProps = pmiSelection->GetProperties();
	try
	{
		//---- Enter your callback code here -----  
		Annotations::Annotation *pmiAnnotation = GetPmiAnnotation(node);
		if (pmiAnnotation)
		{
			pmis.push_back(pmiAnnotation);
		}
		pmiSelectProps->SetTaggedObjectVector("SelectedObjects", pmis);
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	delete pmiSelectProps;
}

//void GWDimList::OnStateChangeCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int state)
//{
//}

//NXString GWDimList::ToolTipTextCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int columnID)
//{
//}

//int GWDimList::ColumnSortCallback(NXOpen::BlockStyler::Tree *tree, int columnID, NXOpen::BlockStyler::Node *node1, NXOpen::BlockStyler::Node *node2)
//{
//}

NXString GWDimList::StateIconNameCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int state)
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
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return stateIcon;
}

//NXOpen::BlockStyler::Tree::BeginLabelEditState GWDimList::OnBeginLabelEditCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int columnID)
//{
//}

//NXOpen::BlockStyler::Tree::EndLabelEditState GWDimList::OnEndLabelEditCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int, NXString editedText)
//{
//}

NXOpen::BlockStyler::Tree::EditControlOption GWDimList::OnEditOptionSelectedCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int columnID, int selectedOptionID, NXString selectedOptionText, NXOpen::BlockStyler::Tree::ControlType type)
{
	if (0 != strcmp(selectedOptionText.GetLocaleText(), oldItemText.GetLocaleText()))
	{
		isModified = true;
	}
	return BlockStyler::Tree::EditControlOptionAccept;
}

NXOpen::BlockStyler::Tree::ControlType GWDimList::AskEditControlCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int columnID)
{
	std::vector<NXOpen::NXString> options;
	int	nItem = 0;

	Annotations::Annotation *pmiAnnotation = GetPmiAnnotation(node);
	if (0 == columnID || NULL == pmiAnnotation)
		return BlockStyler::Tree::ControlTypeNone;
	try
	{
		//---- Enter your callback code here -----  
		if (pmiTextColumn == columnID)
		{
			if (!GW_is_find_attribute(pmiAnnotation, GW_UNLOCK_PMI_TEXT, NXObject::AttributeTypeString))
				return BlockStyler::Tree::ControlTypeNone;
		}
		oldItemText = node->GetColumnDisplayText(columnID);
		GW_split_strings(colunmDataVector[columnID].columnDefaults.GetLocaleText(), ",", options);
		if (!GW_is_in_string_array(options, oldItemText))
			options.push_back(oldItemText);
		for (int i=0; i<(int)options.size(); i++)
		{
			if (0 == strcmp(options[i].GetLocaleText(), oldItemText.GetLocaleText()))
			{
				nItem = i;
				break;
			}
		}
		tree->SetEditOptions(options, nItem); 
		return BlockStyler::Tree::ControlTypeComboBox;                               
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return BlockStyler::Tree::ControlTypeNone;
}

enum MenuID
{
	BreakAssocPart = 0,
	LockPmiText,
	UnlockPmiText
};

void GWDimList::OnMenuCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int columnID )
{
	if (NULL == node)
		return;
	Annotations::Annotation *pmiAnnotation = GetPmiAnnotation(node);
	if (NULL == pmiAnnotation)
		return;
	try
	{
		BlockStyler::TreeListMenu *menu = tree->CreateMenu();
		menu->AddMenuItem(BreakAssocPart, "解除与组件关联");
		menu->AddMenuItem(LockPmiText, "锁定PMI内容");
		menu->AddMenuItem(UnlockPmiText,"解锁PMI内容");
		menu->SetItemIcon(BreakAssocPart,"breaklink");
		menu->SetItemIcon(LockPmiText,"lock");
		menu->SetItemIcon(UnlockPmiText,"unlocked");
		if (NULL == GW_find_associate_part(pmiAnnotation))
			menu->SetItemDisable(BreakAssocPart, true);
		if (GW_is_find_attribute(pmiAnnotation, GW_UNLOCK_PMI_TEXT, NXObject::AttributeTypeString))
			menu->SetItemDisable(UnlockPmiText, true);
		else
			menu->SetItemDisable(LockPmiText, true);
		tree->SetMenu(menu);
		delete menu;           
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}

void GWDimList::OnMenuSelectionCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int menuItemID)
{
	if (NULL == node)
		return;
	Annotations::Annotation *pmiAnnotation = GetPmiAnnotation(node);
	if (NULL == pmiAnnotation)
		return;
	Part *part = GW_find_associate_part(pmiAnnotation);
	switch ((MenuID)menuItemID)
	{
	case BreakAssocPart:
		if (part)
		{
			GW_delete_object_attribute(part->GetTag(), GW_PMI_HANDLE_NAME);
			part->SetAttribute("是否关联", "未关联");
			node->SetColumnDisplayText(0, AskPmiTreeName(pmiAnnotation));
		}
		CleanupItemTexts(node);
		GW_set_object_attribute(pmiAnnotation->GetTag(), GW_BREAK_ASSOC_PART, "1");
		isModified = true;
		break;
	case LockPmiText:
		GW_delete_object_attribute(pmiAnnotation->GetTag(), GW_UNLOCK_PMI_TEXT);
		node->SetColumnDisplayText(pmiTextColumn, GW_ask_pmi_annotation_text(pmiAnnotation));
		node->SetColumnDisplayText(0, AskPmiTreeName(pmiAnnotation));
		isModified = true;
		break;
	case UnlockPmiText:
		GW_set_object_attribute(pmiAnnotation->GetTag(), GW_UNLOCK_PMI_TEXT, "1");
		node->SetColumnDisplayText(0, AskPmiTreeName(pmiAnnotation));
		break;
	}
}

//NXOpen::BlockStyler::Node::DropType GWDimList::IsDropAllowedCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int columnID, NXOpen::BlockStyler::Node *targetNode, int targetColumnID)
//{
//}

//NXOpen::BlockStyler::Node::DragType GWDimList::IsDragAllowedCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int columnID)
//{
//}

//bool GWDimList::OnDropCallback(NXOpen::BlockStyler::Tree *tree, std::vector<NXOpen::BlockStyler::Node *> node, int columnID, NXOpen::BlockStyler::Node *targetNode, int targetColumnID, NXOpen::BlockStyler::Node::DropType dropType, int dropMenuItemId)
//{
//}

//void GWDimList::OnDropMenuCallback(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int columnID, NXOpen::BlockStyler::Node *targetNode, int targetColumnID)
//{
//}

//void GWDimList::OnDefaultAction(NXOpen::BlockStyler::Tree *tree, NXOpen::BlockStyler::Node *node, int columnID)
//{
//}


//------------------------------------------------------------------------------
//Function Name: GetBlockProperties
//Description: Returns the propertylist of the specified BlockID
//------------------------------------------------------------------------------
PropertyList* GWDimList::GetBlockProperties(const char *blockID)
{
	return theDialog->GetBlockProperties(blockID);
}

void GWDimList::InitializePmiTreeList()
{
	Part *workPart(theSession->Parts()->Work());
	std::vector<TaggedObject*> pmis;

	BlockStyler::PropertyList *viewRadioProps = viewRadio->GetProperties();
	BlockStyler::PropertyList *pmiSelectProps = pmiSelection->GetProperties();

	try
	{
		//---- Enter your callback code here -----

		int viewOption = viewRadioProps->GetEnum("Value");
		pmiSelectProps->SetTaggedObjectVector("SelectedObjects", pmis);
		CleanupDimTreeList();

		switch (viewOption)
		{
		case 0:
			{
				std::vector<NXOpen::View *> layoutViews = workPart->Layouts()->Current()->GetViews();
				for (std::vector<NXOpen::View *>::iterator layoutViews_it=layoutViews.begin(); layoutViews_it != layoutViews.end(); layoutViews_it++)
				{
					InsertViewNode(dynamic_cast<NXOpen::ModelingView *>(*layoutViews_it));
				}
			}
			break;
		case 1:
			{
				InsertViewNode(workPart->ModelingViews()->WorkView());
			}
			break;
		case 2:
			{
				NXOpen::ModelingViewCollection *modelingViewCollection = workPart->ModelingViews();
				for (NXOpen::ModelingViewCollection::iterator modelingView_it=modelingViewCollection->begin(); modelingView_it != modelingViewCollection->end(); modelingView_it++)
				{
					InsertViewNode(dynamic_cast<NXOpen::ModelingView *>(*modelingView_it));
				}
			}
			break;
		}
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	delete pmiSelectProps;
	delete viewRadioProps;
}

void GWDimList::InsertViewNode(NXOpen::ModelingView *modelingView)
{
	if (NULL == modelingView)
		return;
	try
	{
		//---- Enter your callback code here -----
		BlockStyler::Node *viewNode = pmiTree->CreateNode(modelingView->Name());
		pmiTree->InsertNode(viewNode, NULL, NULL, NXOpen::BlockStyler::Tree::NodeInsertOptionSort);
		viewNode->SetState(GW_START_DIMENSION_ICON);
		viewNode->ScrollTo(0, BlockStyler::Node::ScrollCenter);
		for (std::vector<Annotations::Annotation *>::iterator pmiAnnotationVector_it=pmiAnnotationVector.begin(); pmiAnnotationVector_it != pmiAnnotationVector.end(); pmiAnnotationVector_it++)
		{
			Annotations::Annotation *pmiAnnotation = *pmiAnnotationVector_it;
			std::vector<NXOpen::View *> pmiViews = pmiAnnotation->GetViews();
			if (0 == pmiViews.size())
			{
				AddPmiToViewNode(viewNode, pmiAnnotation);
			}
			else
			{
				for (std::vector<NXOpen::View *>::const_iterator pmiViews_cit=pmiViews.begin(); pmiViews_cit != pmiViews.end(); pmiViews_cit++)
				{
					if ((*pmiViews_cit) == modelingView)
					{
						AddPmiToViewNode(viewNode, pmiAnnotation);
					}
				}
			}
		}
		viewNode->Expand(BlockStyler::Node::ExpandOptionExpand);
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}

void GWDimList::AddPmiToViewNode(BlockStyler::Node *viewNode, Annotations::Annotation *pmiAnnotation)
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
		for (int i=1; i<(int)(colunmDataVector.size()); i++)
		{
			if (pmiTextColumn == i)
			{
				if (GW_is_find_attribute(itemData.pmiAnnotation, GW_UNLOCK_PMI_TEXT, NXObject::AttributeTypeString) &&
					GW_is_find_attribute(itemData.pmiAnnotation, colunmDataVector[i].columnTitle, NXObject::AttributeTypeString))
				{
					itemData.pmiNode->SetColumnDisplayText(i, itemData.pmiAnnotation->GetStringAttribute(colunmDataVector[i].columnTitle));
				}
				else
				{
					itemData.pmiNode->SetColumnDisplayText(i, GW_ask_pmi_annotation_text(pmiAnnotation));
				}
			}
			else if (GW_is_find_attribute(itemData.pmiAnnotation, colunmDataVector[i].columnTitle, NXObject::AttributeTypeString))
			{
				itemData.pmiNode->SetColumnDisplayText(i, itemData.pmiAnnotation->GetStringAttribute(colunmDataVector[i].columnTitle));
			}
		}
		Annotations::Dimension *pmiDimension = dynamic_cast<Annotations::Dimension *>(itemData.pmiAnnotation);
		if (pmiDimension)
		{
			UF_OBJ_ask_type_and_subtype(itemData.pmiAnnotation->GetTag(), &type, &subtype);
			itemData.pmiNode->SetState(GW_START_DIMENSION_ICON+subtype);
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
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}

NXString GWDimList::AskPmiTreeName(Annotations::Annotation *pmiAnnotation)
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

void GWDimList::CleanupDimTreeList()
{
	try
	{
		//---- Enter your callback code here -----  
		BlockStyler::Node *rootNote = pmiTree->RootNode();
		while (rootNote)
		{
			pmiTree->DeleteNode(rootNote);
			rootNote = pmiTree->RootNode();
		}
		itemDataVector.clear();
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}

BlockStyler::Node* GWDimList::GetPmiNode(const Annotations::Annotation *pmiAnnotation)
{
	for (std::vector<ITEMDATA>::const_iterator itemDataVector_cit=itemDataVector.begin(); itemDataVector_cit != itemDataVector.end(); itemDataVector_cit++)
	{
		if (itemDataVector_cit->pmiAnnotation == pmiAnnotation)
			return itemDataVector_cit->pmiNode;
	}
	return NULL;
}

Annotations::Annotation* GWDimList::GetPmiAnnotation(const BlockStyler::Node *pmiNode)
{
	for (std::vector<ITEMDATA>::const_iterator itemDataVector_cit=itemDataVector.begin(); itemDataVector_cit != itemDataVector.end(); itemDataVector_cit++)
	{
		if (itemDataVector_cit->pmiNode == pmiNode)
			return itemDataVector_cit->pmiAnnotation;
	}
	return NULL;
}

void GWDimList::InitializePmiAnnotationVector()
{
	Session *theSession = NXOpen::Session::GetSession();
	Part	*workPart(theSession->Parts()->Work());
	UI		*theUI = UI::GetUI();
	int		type, subtype;

	pmiAnnotationVector.clear();
	try
	{
		NXOpen::Annotations::PmiCollection *pmiCollection = workPart->PmiManager()->Pmis();
		for (NXOpen::Annotations::PmiCollection::iterator pmiCollection_it=pmiCollection->begin(); pmiCollection_it!=pmiCollection->end(); pmiCollection_it++)
		{
			std::vector<NXOpen::Annotations::Annotation *> instances = (*pmiCollection_it)->GetDisplayInstances();
			for (std::vector<NXOpen::Annotations::Annotation *>::iterator instances_it=instances.begin(); instances_it!=instances.end(); instances_it++)
			{
				Annotations::Annotation *pmiAnnotation = dynamic_cast<Annotations::Annotation *>(*instances_it);
				if (NULL == pmiAnnotation)
					continue;
				tag_t pmi_tag = (*instances_it)->GetTag();
			

			 
		 	pmiAnnotationVector.push_back(pmiAnnotation);
			/*
				Annotations::Dimension *pmiDimension = dynamic_cast<Annotations::Dimension *>(pmiAnnotation);
				if (pmiDimension)
				{
					UF_OBJ_ask_type_and_subtype(pmi_tag, &type, &subtype);
					if (UF_dimension_type == type && UF_dim_ordinate_origin_subtype != subtype)
					{
						pmiAnnotationVector.push_back(pmiAnnotation);
						continue;
					}
				}
				Annotations::SurfaceFinish *pmiSurfaceFinish = dynamic_cast<Annotations::SurfaceFinish *>(pmiAnnotation);
				if (pmiSurfaceFinish)
				{
					pmiAnnotationVector.push_back(pmiAnnotation);
					continue;
				}
				Annotations::Fcf *fcf = dynamic_cast<Annotations::Fcf *>(pmiAnnotation);
				if (fcf)
				{
					pmiAnnotationVector.push_back(pmiAnnotation);
					continue;
				}
				*/
			}
		}
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}

void GWDimList::SelectCurrentAnnotation()
{
	std::vector<TaggedObject*> pmis;
	if (NULL == currentAnnotation)
		return;
	pmis.push_back(currentAnnotation);
	BlockStyler::PropertyList *pmiSelectProps = pmiSelection->GetProperties();
	pmiSelectProps->SetTaggedObjectVector("SelectedObjects", pmis);
	delete pmiSelectProps;

	BlockStyler::Node *pmiNode = GetPmiNode(currentAnnotation);
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

Part* GWDimList::CreateAssociatePart(char *part_number, const Annotations::Annotation *pmiAnnotation)
{
	Part *part = NULL;

	tag_t disp_part = UF_PART_ask_display_part();
	if (NULL_TAG == disp_part)
		return NULL;
	tag_t part_tag = GW_ugmgr_create_component_part(disp_part, part_number, "001", "master", part_number, strPartType.c_str());
	if (part_tag)
	{
		GW_load_part(part_tag);
		GW_set_object_attribute(part_tag, GW_PMI_HANDLE_NAME, UF_TAG_ask_handle_of_tag(pmiAnnotation->GetTag()));
		part = dynamic_cast<Part *>(NXOpen::NXObjectManager::Get(part_tag));
	}
	return part;
}

bool GWDimList::InitializeColumnDatas(const char *config_file)
{
	std::vector<NXString>	stringArray;
	COLUMNDATA	columnData;
	char		buffer[GW_MAX_STRING_LEN];

	colunmDataVector.clear();
    FILE *fp = fopen(config_file, "r");
    if (fp == NULL)
        return false;
	while (NULL != fgets(buffer, GW_MAX_STRING_LEN, fp))
	{
		if (GW_split_strings(buffer, ";", stringArray) >= 3)
		{
			columnData.columnTitle = stringArray[0];
			columnData.columnWidth = atoi(stringArray[1].GetLocaleText());
			columnData.columnDefaults = stringArray[2];
			colunmDataVector.push_back(columnData);
		}
	}
    fclose(fp);
	return true;
}

bool GWDimList::InitializeDialogDefaults(const char *config_file)
{
	std::vector<NXString>	splitArray, stringArray;
	std::string	leftString, rightString, folderString;
	char		buffer[GW_MAX_STRING_LEN];

	ugmgrFolderPath.clear();
	FILE *fp = fopen(config_file, "r");
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
			else if ("UGMGR_DEFLULT_FOLDER" == leftString)
			{
				GW_split_strings(rightString.c_str(), "\\", stringArray);
				for (std::vector<NXString>::iterator stringArray_it=stringArray.begin(); stringArray_it!=stringArray.end(); stringArray_it++)
				{
					folderString = stringArray_it->GetLocaleText();
					if (folderString.empty())
						continue;
					ugmgrFolderPath.push_back(folderString);
				}
			}
			else if ("UGMGR_PART_TYPE" == leftString)
			{
				std::vector<NXString>	itemArray, attrArray;
				char szValue[UF_ATTR_MAX_STRING_LEN];
				tag_t work_part = UF_ASSEM_ask_work_part();
				if (GW_ask_object_attribute(work_part, "DB_PART_TYPE", szValue))
				{
					GW_split_strings(rightString.c_str(), ";", itemArray);
					for (std::vector<NXString>::iterator itemArray_it=itemArray.begin(); itemArray_it!=itemArray.end(); itemArray_it++)
					{
						if (GW_split_strings(itemArray_it->GetLocaleText(), "&", attrArray) >= 2)
						{
							leftString = attrArray[0].GetLocaleText();
							rightString = attrArray[1].GetLocaleText();
							GW_trim(leftString);
							GW_trim(rightString);
							if (rightString.empty())
								continue;
							if (leftString == szValue)
							{
								strPartType = rightString;
								break;
							}
						}
					}
				}
			}
		}
	}
	fclose(fp);
	return true;
}

void GWDimList::RemoveUnusedComponents()
{
	std::vector<tag_t>	occArray;
	tag_t	previous_work_part;
	char	object_handle[UF_ATTR_MAX_STRING_LEN];

	tag_t occ_root = UF_ASSEM_ask_root_part_occ(UF_PART_ask_display_part());
	GW_cycle_components(occ_root, occArray);
	for (std::vector<tag_t>::const_iterator occArray_cit=occArray.begin(); occArray_cit != occArray.end(); occArray_cit++)
	{
		tag_t occ_part = *occArray_cit;
		if (!GW_ask_object_attribute(occ_part, GW_PMI_HANDLE_NAME, object_handle))
			continue;
		tag_t object_tag = UF_TAG_ask_tag_of_handle(object_handle);
		if (NULL_TAG == object_tag || UF_OBJ_ALIVE != UF_OBJ_ask_status(object_tag))
		{
			tag_t pro_part = UF_ASSEM_ask_prototype_of_occ(occ_part);
			pro_part = GW_load_part(pro_part);
			UF_CALL(UF_ASSEM_set_work_part_quietly(pro_part, &previous_work_part));
			GW_set_object_attribute(pro_part, "是否废弃", "是");
			UF_CALL(UF_PART_save());
			UF_CALL(UF_ASSEM_set_work_part_quietly(previous_work_part, &pro_part));
			tag_t ins_part = UF_ASSEM_ask_inst_of_part_occ(occ_part);
			UF_CALL(UF_ASSEM_remove_instance(ins_part));
		}
	}
}

void GWDimList::CleanupItemTexts(BlockStyler::Node *pmiNode)
{
	try
	{
		//---- Enter your callback code here -----
		int nColumnCount = (int)(colunmDataVector.size());
		for (int nCol=1; nCol<nColumnCount; nCol++)
		{
			if (nCol == pmiTextColumn)
				continue;
			pmiNode->SetColumnDisplayText(nCol, "");
		}
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}

bool GWDimList::IsEmptyItemTexts(BlockStyler::Node *pmiNode)
{
	try
	{
		//---- Enter your callback code here -----
		int nColumnCount = (int)(colunmDataVector.size());
		for (int nCol=1; nCol<nColumnCount; nCol++)
		{
			if (nCol == pmiTextColumn)
				continue;
			NXOpen::NXString itemText = pmiNode->GetColumnDisplayText(nCol);
			if (strlen(itemText.GetLocaleText()))
				return false;
		}
	}
	catch(exception& ex)
	{
		//---- Enter your exception handling code here -----
		GWDimList::theUI->NXMessageBox()->Show("Error Message", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return true;
}