// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/views/login_view.h"

#include <string>

#include "app/l10n_util.h"
#include "base/compiler_specific.h"
#include "base/message_loop.h"
#include "grit/generated_resources.h"
#include "views/grid_layout.h"
#include "views/controls/label.h"
#include "views/controls/textfield/textfield.h"
#include "views/standard_layout.h"
#include "views/widget/root_view.h"

static const int kMessageWidth = 320;
static const int kTextfieldStackHorizontalSpacing = 30;

using views::GridLayout;

///////////////////////////////////////////////////////////////////////////////
// LoginView, public:

LoginView::LoginView(const std::wstring& explanation,
                     bool focus_view)
    : username_field_(new views::Textfield),
      password_field_(new views::Textfield(views::Textfield::STYLE_PASSWORD)),
      username_label_(new views::Label(
          l10n_util::GetString(IDS_LOGIN_DIALOG_USERNAME_FIELD))),
      password_label_(new views::Label(
          l10n_util::GetString(IDS_LOGIN_DIALOG_PASSWORD_FIELD))),
      message_label_(new views::Label(explanation)),
      ALLOW_THIS_IN_INITIALIZER_LIST(focus_grabber_factory_(this)),
      login_model_(NULL),
      focus_delayed_(false),
      focus_view_(focus_view) {
  message_label_->SetMultiLine(true);
  message_label_->SetHorizontalAlignment(views::Label::ALIGN_LEFT);
  message_label_->SetAllowCharacterBreak(true);

  // Initialize the Grid Layout Manager used for this dialog box.
  GridLayout* layout = CreatePanelGridLayout(this);
  SetLayoutManager(layout);

  // Add the column set for the information message at the top of the dialog
  // box.
  const int single_column_view_set_id = 0;
  views::ColumnSet* column_set =
      layout->AddColumnSet(single_column_view_set_id);
  column_set->AddColumn(GridLayout::FILL, GridLayout::FILL, 1,
                        GridLayout::FIXED, kMessageWidth, 0);

  // Add the column set for the user name and password fields and labels.
  const int labels_column_set_id = 1;
  column_set = layout->AddColumnSet(labels_column_set_id);
  column_set->AddPaddingColumn(0, kTextfieldStackHorizontalSpacing);
  column_set->AddColumn(GridLayout::LEADING, GridLayout::CENTER, 0,
                        GridLayout::USE_PREF, 0, 0);
  column_set->AddPaddingColumn(0, kRelatedControlHorizontalSpacing);
  column_set->AddColumn(GridLayout::FILL, GridLayout::CENTER, 1,
                        GridLayout::USE_PREF, 0, 0);
  column_set->AddPaddingColumn(0, kTextfieldStackHorizontalSpacing);

  layout->StartRow(0, single_column_view_set_id);
  layout->AddView(message_label_);

  layout->AddPaddingRow(0, kUnrelatedControlLargeVerticalSpacing);

  layout->StartRow(0, labels_column_set_id);
  layout->AddView(username_label_);
  layout->AddView(username_field_);

  layout->AddPaddingRow(0, kRelatedControlVerticalSpacing);

  layout->StartRow(0, labels_column_set_id);
  layout->AddView(password_label_);
  layout->AddView(password_field_);

  layout->AddPaddingRow(0, kUnrelatedControlVerticalSpacing);
}

LoginView::~LoginView() {
  if (login_model_)
    login_model_->SetObserver(NULL);
}

std::wstring LoginView::GetUsername() {
  return username_field_->text();
}

std::wstring LoginView::GetPassword() {
  return password_field_->text();
}

void LoginView::SetModel(LoginModel* model) {
  login_model_ = model;
  if (login_model_)
    login_model_->SetObserver(this);
}

void LoginView::RequestFocus() {
  if (!focus_view_)
    return;

  MessageLoop::current()->PostTask(FROM_HERE,
    focus_grabber_factory_.NewRunnableMethod(&LoginView::FocusFirstField));
}

///////////////////////////////////////////////////////////////////////////////
// LoginView, views::View, views::LoginModelObserver overrides:

void LoginView::ViewHierarchyChanged(bool is_add, View *parent, View *child) {
  if (is_add && child == this && focus_view_) {
    MessageLoop::current()->PostTask(FROM_HERE,
        focus_grabber_factory_.NewRunnableMethod(&LoginView::FocusFirstField));
  }
}

void LoginView::NativeViewHierarchyChanged(bool attached,
                                           gfx::NativeView native_view,
                                           views::RootView* root_view) {
  if (focus_delayed_ && attached && focus_view_) {
    focus_delayed_ = false;
    MessageLoop::current()->PostTask(FROM_HERE,
        focus_grabber_factory_.NewRunnableMethod(&LoginView::FocusFirstField));
  }
}

void LoginView::OnAutofillDataAvailable(const std::wstring& username,
                                        const std::wstring& password) {
  if (username_field_->text().empty()) {
    username_field_->SetText(username);
    password_field_->SetText(password);
    username_field_->SelectAll();
  }
}

///////////////////////////////////////////////////////////////////////////////
// LoginView, private:

void LoginView::FocusFirstField() {
  DCHECK(focus_view_);
  if (GetFocusManager()) {
    username_field_->RequestFocus();
  } else {
    // We are invisible - delay until it is no longer the case.
    focus_delayed_ = true;
  }
}