<?php

/**
 * @file
 * This file display CSV Product uploader form
 */

/**
 * Implements Product CSV uploder form
 * @param type $form
 * @param type $form_state
 * @return string
 */
function product_uploader_form($form, &$form_state) {
  $markup = l(t('Click Here'), drupal_get_path('module', 'intelli_ct_type') . '/' . 'upload_files/product_uploader.csv') . ' To download the template<br />';
  $form['wrap_data'] = array(
    '#markup' => $markup,
  );
  $form['file'] = array(
    '#type' => 'file',
    '#title' => t('Choose a CSV file'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Upload'),
    '#validate' => array('product_uploader_validate'),
  );
  return $form;
}

/**
 * Implements validation of product_uploader_form form
 * @param type $form
 * @param type $form_state
 */
function product_uploader_validate($form, &$form_state) {
  $validators = array(
    'file_validate_extensions' => array('csv CSV'),
  );
  $file = file_save_upload('file', $validators, 'public://');
  if ($file) {
    $csv_dir = 'public://uploadcsvfile';
    $directory_exists = file_prepare_directory($csv_dir, FILE_CREATE_DIRECTORY);
    if ($directory_exists) {
      $destination = $csv_dir . '/';
      $file_moved = file_move($file, $destination);
      if ($file_moved) {
        $form_state['values']['uploadcsvfile'] = $file_moved;
      }
      else {
        form_set_error('file', t('Unable to copy upload file to !dest', array('!dest' => $destination)));
      }
    }
    if (isset($form_state['values']['uploadcsvfile'])) {
      $handle = fopen($form_state['values']['uploadcsvfile']->uri, 'r');
      if ($handle) {
        $line = fgetcsv($handle);
        fclose($handle);
      }
      else {
        form_set_error('file', t('Unable to read uploaded file !filepath', array('!filepath' => $form_state['values']['csvupload']->uri)));
      }
    }
  }
  elseif ($file === NULL) {
    form_set_error('file', 'Please select a file to upload');
  }
}

/**
 * Implements Submit function for product_uploader_form form
 * @param type $form
 * @param type $form_state
 */
function product_uploader_form_submit($form, &$form_state) {
  $file = $form_state['values']['uploadcsvfile'];
  $batch = array(
    'title' => t('Upload products'),
    'operations' => array(
      array('product_uploader_details', array($file)),
    ),
    'progress_message' => t('Processing the data...'),
    'error_message' => t('Error!'),
    'finished' => 'product_uploader_finish',
    'file' => drupal_get_path('module', 'intelli_ct_type') . '/intelli_ct_type.admin.inc',
  );
  batch_set($batch); //initiate batch process
  batch_process('admin/product-uploader'); //call batch process
}
