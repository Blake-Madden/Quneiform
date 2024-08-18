/////////////////////////////////////////////////////////////////////////////
// Name:        datamodel.cpp
// Author:      Blake Madden
// Copyright:   (c) 2021-2024 Blake Madden
// License:     3-Clause BSD license
// SPDX-License-Identifier: BSD-3-Clause
///////////////////////////////////////////////////////////////////////////////

#include "datamodel.h"

//------------------------------------------------------
I18NResultsTreeModel::I18NResultsTreeModel()
    {
    m_root = new I18NResultsTreeModelNode(nullptr, wxString{});
    }

//------------------------------------------------------
void I18NResultsTreeModel::AddRow(const wxString& fileName, const wxString& warningId,
                                  const wxString& issue, const wxString& explanation,
                                  const int line, const int column)
    {
    wxASSERT(m_root);

    auto newFile = new I18NResultsTreeModelNode(m_root, fileName);
    m_root->Append(newFile);

        // notify control
        {
        wxDataViewItem child(reinterpret_cast<void*>(newFile));
        wxDataViewItem parent(reinterpret_cast<void*>(m_root));
        ItemAdded(parent, child);
        }

    I18NResultsTreeModelNode* child_node =
        new I18NResultsTreeModelNode(newFile, warningId, issue, explanation, line, column);
    newFile->Append(child_node);

        // notify control
        {
        wxDataViewItem child(reinterpret_cast<void*>(child_node));
        wxDataViewItem parent(reinterpret_cast<void*>(newFile));
        ItemAdded(parent, child);
        }
    }

//------------------------------------------------------
void I18NResultsTreeModel::Delete(const wxDataViewItem& item)
    {
    I18NResultsTreeModelNode* node = (I18NResultsTreeModelNode*)item.GetID();
    if (node == nullptr) // happens if item.IsOk()==false
        {
        return;
        }

    wxDataViewItem parent(node->GetParent());
    if (!parent.IsOk())
        {
        wxASSERT(node == m_root);

        // don't make the control completely empty:
        wxLogError(L"Cannot remove the root item!");
        return;
        }

    // first remove the node from the parent's array of children;
    auto& siblings = node->GetParent()->GetChildren();
    for (auto it = siblings.begin(); it != siblings.end(); ++it)
        {
        if (it->get() == node)
            {
            siblings.erase(it);
            break;
            }
        }

    // notify control
    ItemDeleted(parent, item);
    }

//------------------------------------------------------
void I18NResultsTreeModel::Clear()
    {
    m_root->GetChildren().clear();

    Cleared();
    }

//------------------------------------------------------
int I18NResultsTreeModel::Compare(const wxDataViewItem& item1, const wxDataViewItem& item2,
                                  unsigned int column, bool ascending) const
    {
    wxASSERT(item1.IsOk() && item2.IsOk());
    // should never happen

    if (IsContainer(item1) && IsContainer(item2))
        {
        wxVariant value1, value2;
        GetValue(value1, item1, 0);
        GetValue(value2, item2, 0);

        int res = value1.GetString().Cmp(value2.GetString());
        if (res != 0)
            {
            return res;
            }

        // items must be different
        wxUIntPtr litem1 = reinterpret_cast<wxUIntPtr>(item1.GetID());
        wxUIntPtr litem2 = reinterpret_cast<wxUIntPtr>(item2.GetID());

        return litem1 - litem2;
        }

    return wxDataViewModel::Compare(item1, item2, column, ascending);
    }

//------------------------------------------------------
void I18NResultsTreeModel::GetValue(wxVariant& variant, const wxDataViewItem& item,
                                    unsigned int col) const
    {
    wxASSERT(item.IsOk());

    I18NResultsTreeModelNode* node = reinterpret_cast<I18NResultsTreeModelNode*>(item.GetID());
    switch (col)
        {
    case 0:
        variant = node->m_warningId;
        break;
    case 1:
        variant = node->m_issue;
        break;
    case 2:
        if (node->m_line != -1)
            {
            variant = static_cast<long>(node->m_line);
            }
        break;
    case 3:
        if (node->m_column != -1)
            {
            variant = static_cast<long>(node->m_column);
            }
        break;
    case 4:
        variant = node->m_explaination;
        break;
    default:
        wxLogError(L"I18NResultsTreeModel::GetValue(): wrong column %d", col);
        }
    }

//------------------------------------------------------
bool I18NResultsTreeModel::SetValue(const wxVariant& variant, const wxDataViewItem& item,
                                    unsigned int col)
    {
    wxASSERT(item.IsOk());

    I18NResultsTreeModelNode* node = (I18NResultsTreeModelNode*)item.GetID();
    switch (col)
        {
    case 0:
        node->m_warningId = variant.GetString();
        return true;
    case 1:
        node->m_issue = variant.GetString();
        return true;
    case 2:
        node->m_line = variant.GetLong();
        return true;
    case 3:
        node->m_column = variant.GetLong();
        return true;
    case 4:
        node->m_explaination = variant.GetString();
        return true;
    default:
        wxLogError(L"I18NResultsTreeModel::SetValue(): wrong column");
        }
    return false;
    }

//------------------------------------------------------
wxDataViewItem I18NResultsTreeModel::GetParent(const wxDataViewItem& item) const
    {
    // the invisible root node has no parent
    if (!item.IsOk())
        {
        return wxDataViewItem(nullptr);
        }

    I18NResultsTreeModelNode* node = (I18NResultsTreeModelNode*)item.GetID();

    // "I18NResults" also has no parent
    if (node == m_root)
        {
        return wxDataViewItem(nullptr);
        }

    return wxDataViewItem((void*)node->GetParent());
    }

//------------------------------------------------------
bool I18NResultsTreeModel::IsContainer(const wxDataViewItem& item) const
    {
    // the invisible root node can have children
    // (in our model always "I18NResults")
    if (!item.IsOk())
        {
        return true;
        }

    I18NResultsTreeModelNode* node = reinterpret_cast<I18NResultsTreeModelNode*>(item.GetID());
    return node->IsContainer();
    }

//------------------------------------------------------
unsigned int I18NResultsTreeModel::GetChildren(const wxDataViewItem& parent,
                                               wxDataViewItemArray& array) const
    {
    I18NResultsTreeModelNode* node = reinterpret_cast<I18NResultsTreeModelNode*>(parent.GetID());
    if (!node)
        {
        return GetChildren(wxDataViewItem(m_root), array);
        }

    if (node->GetChildCount() == 0)
        {
        return 0;
        }

    for (const auto& child : node->GetChildren())
        {
        array.Add(wxDataViewItem(child.get()));
        }

    return array.size();
    }