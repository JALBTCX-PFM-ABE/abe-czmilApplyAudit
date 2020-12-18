
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.
*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "optionPage.hpp"
#include "optionPageHelp.hpp"


optionPage::optionPage (QWidget *prnt, OPTIONS *op):
  QWizardPage (prnt)
{
  options = op;
  parent = prnt;

  setTitle (tr ("Options"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap (":/icons/czmilApplyAuditWatermark.png"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);



  QGroupBox *tBox = new QGroupBox (tr ("Tolerance (bins)"), this);
  QVBoxLayout *tBoxLayout = new QVBoxLayout;
  tBox->setLayout (tBoxLayout);
  tBoxLayout->setSpacing (10);

  QLabel *label = new QLabel (tr ("Use the field below to set the waveform X axis tolerance for finding matching returns in the CPF record.  "
                                  "The tolerance is in bins which roughly equates to one nanosecond per bin.  Just to give a rough idea of "
                                  "what this means, light travels about .3 meters in air in one nanosecond.  In water it's about .225 meters. "
                                  "Audit records in the CAF file are read, the corresponding CPF record is read, and then the interest points "
                                  "from the selected channel are checked against the audit file interest points.  If they are within this "
                                  "tolerance they will be marked as manually invalid."));


  label->setWordWrap(true);

  tBoxLayout->addWidget (label);


  tolerance = new QDoubleSpinBox (this);
  tolerance->setDecimals (2);
  tolerance->setRange (0.01, 3.0);
  tolerance->setSingleStep (0.1);
  tolerance->setValue (options->tolerance);
  tolerance->setWrapping (true);
  tolerance->setToolTip (tr ("Set the waveform X axis tolerance"));
  tolerance->setWhatsThis (toleranceText);
  tBoxLayout->addWidget (tolerance);


  vbox->addWidget (tBox);


  QGroupBox *rBox = new QGroupBox (tr ("Matching return numbers"), this);
  QVBoxLayout *rBoxLayout = new QVBoxLayout;
  rBox->setLayout (rBoxLayout);
  rBoxLayout->setSpacing (10);

  QLabel *rlabel = new QLabel (tr ("Check the box below to check for matching return numbers when the number of returns for a channel "
                                   "in the CPF file matches the number of returns for the channel in the CAF record.<br><br>"
                                   "<b>Use with caution.  This may remove valid data.</b>"));


  rlabel->setWordWrap(true);

  rBoxLayout->addWidget (rlabel);

  returnCheck = new QCheckBox (tr ("Invalidate matching return numbers"), this);
  returnCheck->setToolTip (tr ("If checked, invlidate matching returns"));
  returnCheck->setWhatsThis (returnCheckText);
  returnCheck->setChecked (options->return_check);
  rBoxLayout->addWidget (returnCheck);

  vbox->addWidget (rBox);


  registerField ("tolerance", tolerance, "value", "valueChanged");
  registerField ("returnCheck", returnCheck);
}
