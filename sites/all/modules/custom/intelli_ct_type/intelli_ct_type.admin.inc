<?php

/**
 * @file
 * This is admin config. file, where we write page callback function for Display Product Data Url and Product Uploader function
 */

/**
 * This is callback function Display Product Data
 * @global type $base_url
 * @return theme_table
 */
function display_product_data() {
  global $base_url;
  $destination = drupal_get_destination();

  //fetch nodes of Product cintent type
  $q = db_select('node', 'n')->extend('PagerDefault')->extend('TableSort');
  $nids = $q->fields('n', array('nid'))
      ->condition('type', 'product')
      ->limit(20)
      ->execute()
      ->fetchCol();

  $languages = language_list();
  $nodes = node_load_multiple($nids); //load multiple nodes
  //mentioned theme table header section
  $header = array(
    'title' => array('data' => t('Title'), 'field' => 'n.title'),
    'type' => array('data' => t('Type'), 'field' => 'n.type'),
    'author' => t('Author'),
    'status' => array('data' => t('Status'), 'field' => 'n.status'),
    'changed' => array('data' => t('Updated'), 'field' => 'n.changed', 'sort' => 'desc')
  );
  $header['operations'] = array('data' => t('Operations'));
  $options = array();
  foreach ($nodes as $node) {
    $langcode = entity_language('node', $node);
    $uri = entity_uri('node', $node);
    if ($langcode != LANGUAGE_NONE && isset($languages[$langcode])) {
      $uri['options']['language'] = $languages[$langcode];
    }
    //reder nid details
    $options[$node->nid] = array(
      'title' => array(
        'data' => array(
          '#type' => 'link',
          '#title' => $node->title,
          '#href' => $uri['path'],
          '#options' => $uri['options'],
          '#suffix' => ' ' . theme('mark', array('type' => node_mark($node->nid, $node->changed))),
        ),
      ),
      'type' => check_plain(node_type_get_name($node)),
      'author' => theme('username', array('account' => $node)),
      'status' => $node->status ? t('published') : t('not published'),
      'changed' => format_date($node->changed, 'short'),
    );
    //add edit and delete operations link
    $operations = array();
    $operations['edit'] = array(
      'title' => t('edit'),
      'href' => 'node/' . $node->nid . '/edit',
      'query' => $destination,
    );
    $operations['delete'] = array(
      'title' => t('delete'),
      'href' => 'node/' . $node->nid . '/delete',
      'query' => $destination,
    );
    $options[$node->nid]['operations'] = array();
    $options[$node->nid]['operations'] = array(
      'data' => array(
        '#theme' => 'links__node_operations',
        '#links' => $operations,
        '#attributes' => array('class' => array('links', 'inline')),
      ),
    );
  }
  //display add content url
  $add_content_url = '<a href="' . $base_url . '/node/add/product">+ Add content</a><br><br>';
  //render theme table with header and footer
  $output .= theme('table', array(
    'header' => $header,
    'rows' => $options,
    'attributes' => array('width' => '100%')
  ));
  //display pager for more than 20 products
  $output .= theme('pager');
  return $add_content_url . $output;
}

/**
 * This is batch operation function for Product Uploader
 * @param type $file_obj
 * @param type $context
 */
function product_uploader_details($file_obj, &$context) {
  $fp = file($file_obj->uri);
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 1;
    $context['sandbox']['max'] = count($fp) - 1;
    $context['results']['success'] = 0;
    $context['results']['error'] = 0;
  }
  $file = new SplFileObject($file_obj->uri);
  $header = $file->current(); //Always gets the header.
  $progress = $context['sandbox']['progress'];
  $max = $context['sandbox']['max'];
  if ($max == 0) {
    drupal_set_message(t('Please enter data in sheet'), 'error');
  }
  for ($i = 0; ($i < 100) && (($i + $progress) <= $max); $i++) {
    $file->seek($i + $progress);
    $data = str_getcsv($file->current());
    $head = explode(',', $header);
    $error = FALSE;
    foreach ($data as $key => $value) {
      $data[$key] = trim($value);
      if ((is_null($data[$key]) || empty($data[$key]))) {
        $context['results']['error'] ++;
        $error_string = $head[$key] . ' is empty at row ' . ($i + $progress);
        $context['results']['error_array'][$i + $progress] .= $error_string;
        $context['sandbox']['progress'] = $i + $progress;
        continue 2;
      }
    }
    $product_title = trim($data[0]);
    $description = trim($data[1]);
    $selling_price = trim($data[2]);
    $offer_price = trim($data[3]);
    $product_image_name = trim($data[4]);
    $sku = trim($data[5]);
    $product_tag = explode(',', $data[6]);
    $product_brand = trim($data[7]);
    $flag = 0;
    $node = new stdClass();  // Create a new node object
    $node->type = 'product';  // Content type
    $node->language = LANGUAGE_NONE;  // Or e.g. 'en' if locale is enabled
    node_object_prepare($node);  //Set some default values
    $node->title = $product_title;
    $node->intelli_ct_type_description[LANGUAGE_NONE][0]['value'] = $description;
    $node->intelli_ct_type_sell_price[LANGUAGE_NONE][0]['value'] = $selling_price;
    $node->intelli_ct_type_offer_price[LANGUAGE_NONE][0]['value'] = $offer_price;
    // Create image File object and associate with Image field.
    $filename = $product_image_name;

    $filename = match_data($filename, 'sites/default/files/product_images/');
    if (!empty($filename)) {
      $filepath = drupal_realpath($filename);
      $flag = 1;
    }
    else {
      $filename = match_data($product_image_name, 'sites/default/files/');
      if (!empty($filename)) {
        $filepath = drupal_realpath($filename);
        $flag = 2;
      }
    }
    if ($flag == 1 || $flag == 2) {
      $b = strripos($filename, '/');
      $c = substr($filename, $b + 1);
      $fid = _check_image_exist($c);
      if (!empty($fid)) {
        if ($flag == 1) {
          file_unmanaged_move($filepath, 'public://', FILE_EXISTS_REPLACE);
        }
        $img_file = file_load($fid[0]);
      }
      else {
        $img_file = (object) array(
              'uid' => 1,
              'uri' => $filepath,
              'filemime' => file_get_mimetype($filepath),
              'status' => 1,
              'filename' => $c,
        );
        $img_file = file_move($img_file, 'public://');
      }
//Clear all image styles of the product so they will be rebuilt
      image_path_flush('public://' . $c);
    }
    //$file = file_load(3);
    $node->intelli_ct_type_product_image[LANGUAGE_NONE][0] = (array) $img_file;
    $node->intelli_ct_type_product_sku['und'][0]['value'] = $sku;
    $tid_result = _check_tid_exist($product_tag);
    $node->intelli_ct_type_product_tag[$node->language] = $tid_result;
    $node->intelli_ct_type_product_brand[LANGUAGE_NONE][0]['value'] = $product_brand;
    node_save($node);
    $context['results']['success'] ++;
    $context['sandbox']['progress'] = $i + $progress;
  }
  $rows_remain = $context['sandbox']['max'] - $context['sandbox']['progress'];
  $context['message'] = t($rows_remain . ' Remaining.');
  if ($context['sandbox']['progress'] < $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
  else {
    $context['finished'] = 1;
  }
  $context['results']['fid'] = $file_obj->fid;
}

/**
 * Batch operation finish function
 * @param type $success
 * @param type $results
 * @param type $operations
 */
function product_uploader_finish($success, $results, $operations) {
  if ($success) {
    drupal_set_message(t('Records updated successfully'));
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array('%error_operation' => $error_operation[0], '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  }
}

/**
 * This function is used to check whether the particular file fid exixt in db
 * @param type $file_name as file name
 * @return fid
 */
function _check_image_exist($file_name) {
  $query = db_select('file_managed', 'fm');
  $query->fields('fm', array('fid'));
  $query->condition('fm.filename', $file_name);
  return $query->execute()->fetchCol();
}

/**
 * This function to check uploaded image name with existing image present in directory
 * @param type $file_name
 * @param type $dirname
 * @return boolean
 */
function match_data($file_name, $dirname) {
  $flag = FALSE;
  foreach (glob($dirname . "*.jpg") as $filename) {
    $b = strripos($filename, '/');
    $c = substr($filename, $b + 1);
    $a = strstr($c, ".jpg", TRUE);
    if ($a == $file_name) {
      return $filename;
    }
  }
  return $flag;
}

/**
 * This function is used to check whether tags mentioned in csv is present or mot, If present then return tid 
 * else create new term under tags vocabulary
 * @param type $product_tag as Term Name
 * @return tid array
 */
function _check_tid_exist($product_tag) {
  $return = array();
  foreach ($product_tag as $key => $value) {
    $tid_obj = taxonomy_get_term_by_name($value, 'tags');
    if (!empty($tid_obj)) {
      $tid_array = array('tid' => array_keys($tid_obj));
    }
    else {
      $vid = taxonomy_vocabulary_machine_name_load('tags')->vid;
      $newterm = new stdClass();
      $newterm->name = $value;
      $newterm->vid = $vid;
      taxonomy_term_save($newterm);
      $tid_array = array('tid' => $newterm->tid);
    }
    $return[] = $tid_array;
  }
  return $return;
}
