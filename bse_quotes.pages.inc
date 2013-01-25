<?php 
/**
 * @file
 * Important user functions for the BSE Quotes module.
 */

function bse_quotes() {

  $items = array();

  $query = db_select('bse_quotes', 'bse_quotes')
  ->condition('status', 1)
  ->range(0, 20)
  ->fields('bse_quotes', array(
      'id',
      'stock_code',
      'stock_name',
      'status',
  ));

  $results = $query->execute();

  while ($record = $results->fetchAssoc()) {
    $stock_quote[$record['stock_code']] = get_stock_quote($record['stock_code']);
  }

  return theme('page-quote', array('quotes' => $stock_quote));

}