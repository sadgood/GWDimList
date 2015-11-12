#ifndef GWDIMFUNC_H_INCLUDED
#define GWDIMFUNC_H_INCLUDED

//#define DEBUG

#include <iostream>
#include <stdio.h> 
#include <stdlib.h>
#include <io.h>
#include <vector>
#include <direct.h>
#include <math.h>

//UGÍ·ÎÄ¼þ
#include <uf.h>
#include <uf_defs.h>
#include <uf_exit.h>
#include <uf_ui.h>
#include <uf_assem.h>
#include <uf_part.h>
#include <uf_clone.h>
#include <uf_obj.h>
#include <uf_modl.h>
#include <uf_drf.h>
#include <uf_draw.h>
#include <uf_vec.h>
#include <uf_eval.h>
#include <uf_evalsf.h>
#include <uf_so.h>
#include <uf_csys.h>
#include <uf_mtx.h>
#include <uf_curve.h>
#include <uf_udobj.h>
#include <uf_ugmgr.h>
#include <uf_view.h>

#include <NXOpen/Annotations_Annotation.hxx>
#include <NXOpen/Annotations_AnnotationManager.hxx>
#include <NXOpen/Annotations_AssociatedObject.hxx>
#include <NXOpen/Annotations_Dimension.hxx>
#include <NXOpen/Annotations_DimensionCollection.hxx>
#include <NXOpen/Annotations_Fcf.hxx>
#include <NXOpen/Annotations_FeatureControlFrameBuilder.hxx>
#include <NXOpen/Annotations_FeatureControlFrameDataBuilder.hxx>
#include <NXOpen/Annotations_FeatureControlFrameDataBuilderList.hxx>
#include <NXOpen/Annotations_IPmi.hxx>
#include <NXOpen/Annotations_OriginBuilder.hxx>
#include <NXOpen/Annotations_PlaneBuilder.hxx>
#include <NXOpen/Annotations_PmiAttributeCollection.hxx>
#include <NXOpen/Annotations_PmiCollection.hxx>
#include <NXOpen/Annotations_PmiManager.hxx>
#include <NXOpen/Annotations_PmiPerpendicularDimension.hxx>
#include <NXOpen/Annotations_Pmi.hxx>
#include <NXOpen/Annotations_SearchModelViewBuilder.hxx>
#include <NXOpen/Annotations_SearchModelViewCollection.hxx>
#include <NXOpen/Annotations_SurfaceFinish.hxx>
#include <NXOpen/Annotations_SurfaceFinishBuilder.hxx>
#include <NXOpen/BlockStyler_UIBlock.hxx>
#include <NXOpen/BlockStyler_BlockDialog.hxx>
#include <NXOpen/BlockStyler_PropertyList.hxx>
#include <NXOpen/BlockStyler_UIBlock.hxx>
#include <NXOpen/BlockStyler_BlockDialog.hxx>
#include <NXOpen/BlockStyler_PropertyList.hxx>
#include <NXOpen/BlockStyler_Tree.hxx>
#include <NXOpen/BlockStyler_Node.hxx>
#include <NXOpen/BlockStyler_Label.hxx>
#include <NXOpen/BlockStyler_Group.hxx>
#include <NXOpen/BlockStyler_Enumeration.hxx>
#include <NXOpen/BlockStyler_SelectObject.hxx>
#include <NXOpen/Body.hxx>
#include <NXOpen/Callback.hxx>
#include <NXOpen/CoordinateSystem.hxx>
#include <NXOpen/Direction.hxx>
#include <NXOpen/DirectionCollection.hxx>
#include <NXOpen/Edge.hxx>
#include <NXOpen/Face.hxx>
#include <NXOpen/FaceAdjacentRule.hxx>
#include <NXOpen/FaceConnectedBlendRule.hxx>
#include <NXOpen/FaceDumbRule.hxx>
#include <NXOpen/FaceRibFacesRule.hxx>
#include <NXOpen/FaceTangentRule.hxx>
#include <NXOpen/Features_Brep.hxx>
#include <NXOpen/Features_AngularDim.hxx>
#include <NXOpen/Features_AngularDimBuilder.hxx>
#include <NXOpen/Features_Extrude.hxx>
#include <NXOpen/Features_ExtractFaceBuilder.hxx>
#include <NXOpen/Features_FaceRecognitionBuilder.hxx>
#include <NXOpen/Features_Feature.hxx>
#include <NXOpen/Features_FeatureCollection.hxx>
#include <NXOpen/Features_LinearDimension.hxx>
#include <NXOpen/Features_LinearDimensionBuilder.hxx>
#include <NXOpen/Features_RadialDimension.hxx>
#include <NXOpen/Features_RadialDimensionBuilder.hxx>
#include <NXOpen/GeometricUtilities_EntityUsageInfoList.hxx>
#include <NXOpen/GeometricUtilities_FaceChangeOverflowBehavior.hxx>
#include <NXOpen/GeometricUtilities_OrientXpressBuilder.hxx>
#include <NXOpen/GeometricUtilities_ParentEquivalencyMapList.hxx>
#include <NXOpen/GeometricUtilities_ReplAsstBuilder.hxx>
#include <NXOpen/GeometricUtilities_SaveConstraintsBuilder.hxx>
#include <NXOpen/Layout.hxx>
#include <NXOpen/LayoutCollection.hxx>
#include <NXOpen/ModelingView.hxx>
#include <NXOpen/ModelingViewCollection.hxx>
#include <NXOpen/NXObject.hxx>
#include <NXOpen/NXObjectManager.hxx>
#include <NXOpen/NXException.hxx>
#include <NXOpen/NXMessageBox.hxx>
#include <NXOpen/Part.hxx>
#include <NXOpen/PartCollection.hxx>
#include <NXOpen/Preferences_SessionModeling.hxx>
#include <NXOpen/Preferences_SessionPreferences.hxx>
#include <NXOpen/ScCollector.hxx>
#include <NXOpen/ScCollectorCollection.hxx>
#include <NXOpen/ScRuleFactory.hxx>
#include <NXOpen/SelectDisplayableObject.hxx>
#include <NXOpen/SelectNXObject.hxx>
#include <NXOpen/SelectObject.hxx>
#include <NXOpen/SelectionIntentRule.hxx>
#include <NXOpen/Session.hxx>
#include <NXOpen/TaggedObjectList.hxx>
#include <NXOpen/UI.hxx>
#include <NXOpen/Xform.hxx>
#include <NXOpen/XformCollection.hxx>
#include <NXOpen/Annotations_DatumReferenceBuilderList.hxx>
 
using namespace std;
using namespace NXOpen;
using namespace NXOpen::BlockStyler;

#define UF_CALL(X) (GW_report_error( __FILE__, __LINE__, #X, (X)))

const double TOLERANCE = 0.0001;

#define GW_MAX_STRING_LEN				1024
#define GW_PMI_HANDLE_NAME				"PMI_HANDLE"
#define GW_UNLOCK_PMI_TEXT				"UNLOCK_PMI_TEXT"
#define GW_BREAK_ASSOC_PART				"BREAK_ASSOC_PART"

extern int GW_report_error(char *file, int line, char *call, int irc);
extern void GW_trim(string &str);
extern NXString GW_ask_dimension_string(Annotations::Dimension *pmiDimension);
extern NXString GW_ask_diameter_string(Annotations::Dimension *pmiDimension, NXString dimensionText);
extern NXString GW_ask_radius_string(Annotations::Dimension *pmiDimension, NXString dimensionText);
extern NXString GW_ask_chamfer_string(Annotations::Dimension *pmiDimension, NXString dimensionText);
extern NXString GW_ask_tolerance_string(Annotations::Dimension *pmiDimension);
extern bool GW_ask_tolerance_value(Annotations::Dimension *pmiDimension, double *dUpperValue, double *dLowerValue);
extern bool GW_set_tolerance_value(Annotations::Dimension *pmiDimension, double dUpperValue, double dLowerValue);
extern bool GW_is_find_attribute(NXOpen::NXObject *object, NXString title, NXOpen::NXObject::AttributeType type);
extern Annotations::Pmi *GW_ask_annotation_pmi(Annotations::Annotation *annotation);
extern bool GW_is_same_double(double data1, double data2);
extern void GW_trim_tail_zero(char *number_str);
extern int GW_split_strings(const char *szInput, const char *szSplit, std::vector<NXString> &stringArray);
extern void GW_trim_string_right(char *szString, char cChar);
extern NXString GW_ask_annotation_name(Annotations::Annotation *pmiAnnotation);
extern tag_t GW_ugmgr_create_part(char *part_number, char *part_revision, char *part_file_type, char* part_file_name);
extern tag_t GW_ugmgr_create_component_part(tag_t parent_part, char *part_number, char *part_revision, char *part_file_type, char* part_file_name, const char* part_type);
extern UF_UGMGR_tag_t GW_ugmgr_find_folder(const std::vector<std::string> &folderPath);
extern void GW_cycle_components(tag_t occ_parent, std::vector<tag_t> &occArray);
extern std::string GW_ask_component_name(tag_t occ_part);
extern bool GW_is_in_object_array(const std::vector<tag_t> objectArray, tag_t object_tag);
extern bool GW_is_in_string_array(const std::vector<NXString> stringArray, NXString strInput);
extern int GW_ask_parts_in_assembly(std::vector<tag_t> &partArray);
extern int GW_ask_parts_in_session(std::vector<tag_t> &partArray);
extern int GW_ask_part_names_in_session(std::vector<NXString> &nameArray);
extern void GW_set_object_attribute(tag_t object_tag, const char *szTitle, const char *szValue);
extern bool GW_ask_object_attribute(tag_t object_tag, const char *szTitle, char *szValue);
extern bool GW_delete_object_attribute(tag_t object_tag, const char *szTitle);
extern tag_t GW_load_part(tag_t part_tag);
extern bool GW_ask_new_part_number(std::vector<NXString> &nameArray, char *part_number);
extern bool GW_ask_part_name(tag_t part_tag, char *part_name);
extern int GW_ask_tail_number_string(const char *szInput, char *sTailNum);
extern bool GW_carry_over_integer(std::string &strInteger);
extern NXString GW_ask_pmi_annotation_text(Annotations::Annotation *pmiAnnotation);
extern NXString GW_ask_datum_reference_text(Annotations::DatumReferenceBuilder *datumReferenceBuilder);
extern NXString GW_ask_compound_datum_reference_text(Annotations::CompoundDatumReferenceBuilder *compoundDatumReferenceBuilder);
extern NXString GW_remove_sharp_bracket_text(NXString inputString);
extern int GW_yes_or_no_box(const char *message, const char *title_string);
extern Part* GW_find_associate_part(const Annotations::Annotation *pmiAnnotation);
extern void DRAFT_ask_note_value(tag_t note_tag, char *text);
extern DllExport void  GetGdtChar(tag_t objTag, char * info);
extern int ReplaceStr(char* sSrc, char* sMatchStr, char* sReplaceStr);
#endif /* GWDIMFUNC_H_INCLUDED */
