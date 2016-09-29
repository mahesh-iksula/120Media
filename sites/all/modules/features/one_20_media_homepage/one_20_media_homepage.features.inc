<?php
/**
 * @file
 * one_20_media_homepage.features.inc
 */

/**
 * Implements hook_views_api().
 */
function one_20_media_homepage_views_api($module = NULL, $api = NULL) {
  return array("api" => "3.0");
}

/**
 * Implements hook_image_default_styles().
 */
function one_20_media_homepage_image_default_styles() {
  $styles = array();

  // Exported image style: featured_image.
  $styles['featured_image'] = array(
    'label' => 'featured_image',
    'effects' => array(
      1 => array(
        'name' => 'image_resize',
        'data' => array(
          'width' => 1351,
          'height' => 375,
        ),
        'weight' => 1,
      ),
    ),
  );

  // Exported image style: latest_images.
  $styles['latest_images'] = array(
    'label' => 'latest_images',
    'effects' => array(
      2 => array(
        'name' => 'image_resize',
        'data' => array(
          'width' => 405,
          'height' => 304,
        ),
        'weight' => 1,
      ),
    ),
  );

  return $styles;
}

/**
 * Implements hook_node_info().
 */
function one_20_media_homepage_node_info() {
  $items = array(
    'featured' => array(
      'name' => t('Featured'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  drupal_alter('node_info', $items);
  return $items;
}
